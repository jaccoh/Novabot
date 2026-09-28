import { afterEach, beforeEach, expect, it, vi } from 'vitest';
const h = vi.hoisted(() => ({ online: true, docked: true, publish: vi.fn(), command: vi.fn(), capture: vi.fn(), consume: vi.fn(), measureDock: vi.fn(), manual: undefined as (() => void) | undefined }));
vi.mock('../../mqtt/broker.js', () => ({ isDeviceOnline: () => h.online }));
vi.mock('../../mqtt/mapSync.js', () => ({ publishToExtended: h.publish }));
vi.mock('../../services/frameValidation.js', () => ({ isFrameUnvalidated: () => false }));
vi.mock('../../services/positionTelemetry.js', () => ({ freshPositionState: () => ({ docked: h.docked }), stablePosition: () => ({ x: 0, y: 0 }) }));
vi.mock('../../services/dockPhotoReference.js', () => ({ snapshotDockPose: () => ({ x: 0, y: 0, orientation: 0 }) }));
vi.mock('../../services/reanchorGps.js', () => ({ measureReanchorDock: h.measureDock }));
vi.mock('../../services/scheduleRunner.js', () => ({ disarmEdgeWatch: vi.fn() }));
vi.mock('../../services/mowerMapOperation.js', () => ({
  readMowerMapSnapshot: async () => ({ snapshot_manifest: { file: 'hash' } }),
  withMowerMapOperation: async (sn: string, run: (op: unknown) => unknown) => run({ sn, id: 'lease', command: h.command,
    set onManualControl(value: () => void) { h.manual = value; } }),
}));
vi.mock('../../services/copyAlignment.js', () => ({
  beginCopyAlignment: async () => ({ alignmentId: 'alignment' }), frameSnapshotSignature: () => 'fingerprint',
  captureCopyAlignment: h.capture, consumeCopyAlignment: h.consume,
  getCopyAlignment: () => ({ alignmentId: 'alignment', phase: 'target_first', captures: { source: [] } }),
}));
import { startSourceDockCycle, sourceDockCycle } from '../../services/sourceDockCycle.js';
let id: string, elapsed: number;
const tick = (action: 'pulse' | 'stop' = 'pulse') => sourceDockCycle(id, 'target', 'source', action);
beforeEach(() => {
  vi.useFakeTimers(); elapsed = 0;
  vi.spyOn(performance, 'now').mockImplementation(() => elapsed);
  id = crypto.randomUUID(); h.online = true; h.docked = true;
  h.publish.mockReset(); h.command.mockReset(); h.capture.mockReset(); h.consume.mockReset();
  h.measureDock.mockReset().mockImplementation(async () => ({ dist: 0, latestDist: 0, runtime: { x: 0, y: 0, capture_started: Date.now(), capture_finished: Date.now() - 1 } }));
  h.command.mockImplementation(async (cmd, params) => {
    if (cmd === 'dock_measurement_move') h.docked = params.action === 'dock';
    return { result: 0, protocol: 'dock-measurement-motion-v1', docked: h.docked, frame_fingerprint: 'fingerprint' };
  });
});
afterEach(async () => {
  try { tick('stop'); } catch { /* start was rejected */ }
  elapsed += 310_000;
  await vi.advanceTimersByTimeAsync(6000);
  vi.useRealTimers(); vi.restoreAllMocks();
});
const start = () => startSourceDockCycle(id, 'target', 'source', 'map0');
const advance = async (ms = 100) => { elapsed += ms; await vi.advanceTimersByTimeAsync(ms); };

it('performs two distance-limited reverse steps, validated captures and a confirmed visual dock in order', async () => {
  const events: string[] = [];
  h.command.mockImplementation(async (cmd, params) => {
    events.push(`${cmd}:${params.action}`);
    if (cmd === 'dock_measurement_move') h.docked = params.action === 'dock';
    return { result: 0, protocol: 'dock-measurement-motion-v1', docked: h.docked, frame_fingerprint: 'fingerprint' };
  });
  h.capture.mockImplementation(async (_id, side, owner) => { expect(side).toBe('source'); expect(owner.sn).toBe('source'); events.push('capture'); });
  start(); tick(); await advance();
  expect(tick()).toMatchObject({ phase: 'done', alignment: { phase: 'target_first' } });
  expect(events).toEqual(['dock_measurement_control:arm', 'dock_measurement_move:reverse', 'capture', 'dock_measurement_control:arm', 'dock_measurement_move:reverse', 'capture', 'dock_measurement_control:arm', 'dock_measurement_move:dock']);
  expect(h.command.mock.calls.filter(([cmd]) => cmd === 'dock_measurement_move').map(([, p]) => [p.distance_m, p.from_dock])).toEqual([[.5, true], [.2, false], [0, false]]);
});

it('never moves when the start HTTP response was lost and no browser pulse arrives', async () => {
  start(); await advance(5100);
  expect(tick('stop').phase).toBe('error'); expect(h.command).not.toHaveBeenCalled();
});

it.each(['stop', 'manual', 'disconnect', 'expired-browser', 'measurement-failure'])('does not start the next movement or docking after %s during capture', async kind => {
  h.capture.mockImplementation(async () => {
    if (kind === 'stop') tick('stop');
    if (kind === 'manual') h.manual!();
    if (kind === 'disconnect') h.online = false;
    if (kind === 'expired-browser') elapsed += 5100;
    if (kind === 'measurement-failure') throw new Error('inconsistent marker');
  });
  start(); tick(); await advance();
  expect(tick('stop').phase).toBe('error');
  expect(h.command.mock.calls.filter(([cmd]) => cmd === 'dock_measurement_move')).toHaveLength(1);
  expect(h.consume).toHaveBeenCalledWith('alignment');
});

it('sends a stop for an in-flight movement and retains the cycle until its response', async () => {
  let release!: (value: unknown) => void;
  h.command.mockImplementation(async cmd => cmd === 'dock_measurement_control' ? { result: 0, protocol: 'dock-measurement-motion-v1' } : new Promise(resolve => { release = resolve; }));
  start(); tick(); await advance();
  expect(tick('stop').phase).toBe('reverse_first');
  expect(h.publish).toHaveBeenLastCalledWith('source', { dock_measurement_control: { action: 'stop', motion_id: expect.any(String) } });
  release({ result: 0, protocol: 'dock-measurement-motion-v1' }); await advance();
  expect(tick('stop').phase).toBe('error'); expect(h.capture).not.toHaveBeenCalled();
});

it.each(['legacy', 'timeout', 'no-contact'])('does not report success for %s firmware output', async mode => {
  const good = h.command.getMockImplementation()!;
  h.command.mockImplementation(async (cmd, params) => {
    const result = await good(cmd, params);
    if (mode === 'legacy') return { result: 0 };
    if (cmd === 'dock_measurement_move' && params.action === 'dock') return mode === 'timeout' ? null : { ...result, docked: false };
    return result;
  });
  start(); tick(); await advance(); expect(tick('stop').phase).toBe('error');
});

it('does not duplicate an active start or accept controls from a different mower pair', async () => {
  start(); expect(start().cycleId).toBe(id);
  expect(() => startSourceDockCycle(crypto.randomUUID(), 'target2', 'source', 'map0')).toThrow('already running');
  expect(() => sourceDockCycle(id, 'target2', 'source', 'pulse')).toThrow('Unknown');
  tick('stop'); await advance(); expect(h.command).not.toHaveBeenCalled();
});


it('refuses departure when the independent dock observation disagrees', async () => {
  h.measureDock.mockResolvedValue({ dist: .06, latestDist: .06 });
  start(); tick(); await advance();
  expect(tick('stop').phase).toBe('error'); expect(h.command).not.toHaveBeenCalled();
});

it('discards both captures if the final file or dock check fails', async () => {
  h.measureDock.mockImplementationOnce(async () => ({ dist: 0, latestDist: 0, runtime: { capture_finished: 1 } })).mockRejectedValueOnce(new Error('map files changed'));
  start(); tick(); await advance();
  expect(tick('stop')).toMatchObject({ phase: 'error', error: 'map files changed' });
  expect(tick('stop').alignment).toBeUndefined();
  expect(h.consume).toHaveBeenCalledWith('alignment');
});

it('surfaces a failed native cancellation instead of hiding it behind the operator Stop label', async () => {
  let release!: (value: unknown) => void;
  h.command.mockImplementation(async cmd => cmd === 'dock_measurement_control' ? { result: 0, protocol: 'dock-measurement-motion-v1' } : new Promise(resolve => { release = resolve; }));
  start(); tick(); await advance(); tick('stop');
  release({ result: 1, error: 'visual docking cancellation not confirmed; use the mower STOP button' });
  await advance();
  expect(tick('stop')).toMatchObject({ phase: 'error', error: 'visual docking cancellation not confirmed; use the mower STOP button' });
});
