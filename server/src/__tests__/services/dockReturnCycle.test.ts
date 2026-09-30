import { afterEach, beforeEach, expect, it, vi } from 'vitest';
const h = vi.hoisted(() => ({ online: true, publish: vi.fn(), read: vi.fn(), move: vi.fn(), settle: vi.fn(), command: vi.fn() }));
vi.mock('../../mqtt/broker.js', () => ({ isDeviceOnline: () => h.online }));
vi.mock('../../mqtt/mapSync.js', () => ({ publishToExtended: h.publish }));
vi.mock('../../services/frameValidation.js', () => ({ getFrameRevision: () => 0, isMapInstallPending: () => false }));
vi.mock('../../services/positionTelemetry.js', () => ({ stablePosition: () => ({ x: 0, y: 0 }) }));
vi.mock('../../services/copyAlignment.js', () => ({ frameSnapshotSignature: () => 'frame' }));
vi.mock('../../services/scheduleRunner.js', () => ({ disarmEdgeWatch: vi.fn() }));
vi.mock('../../services/dockMotion.js', async importOriginal => ({
  ...(await importOriginal<typeof import('../../services/dockMotion.js')>()), guardedDockMove: h.move, settleDockMotion: h.settle,
}));
vi.mock('../../services/mowerMapOperation.js', () => ({
  readMowerMapSnapshot: h.read,
  withMowerMapOperation: async (sn: string, run: (operation: unknown) => unknown, reanchor: boolean) =>
    run({ sn, id: 'lease', reanchor, command: h.command }),
}));
import { startDockReturn, dockReturn } from '../../services/dockReturnCycle.js';
import { rememberDockedPose } from '../../services/dockMotion.js';

const LEFT_FROM = { x: 1, y: 2, yaw: .5 };
let id: string, elapsed: number;
const tick = (action: 'pulse' | 'stop' = 'pulse') => dockReturn(id, 'mower', action);
const advance = async (ms: number) => { elapsed += ms; await vi.advanceTimersByTimeAsync(ms); };
beforeEach(() => {
  vi.useFakeTimers(); elapsed = 0; vi.spyOn(performance, 'now').mockImplementation(() => elapsed);
  id = crypto.randomUUID(); h.online = true;
  for (const mock of [h.publish, h.read, h.move, h.settle, h.command]) mock.mockReset();
  h.read.mockResolvedValue({ pos_json: 'unchanged' });
  h.move.mockResolvedValue({}); h.settle.mockResolvedValue({ x: 0, y: 0 });
  rememberDockedPose('mower', LEFT_FROM, 0);
});
afterEach(async () => {
  try { tick('stop'); } catch { /* no active cycle */ }
  elapsed += 130_000; await vi.advanceTimersByTimeAsync(6000);
  vi.useRealTimers(); vi.restoreAllMocks();
});

it('does not move without a live operator pulse', async () => {
  startDockReturn(id, 'mower');
  await advance(5100);
  expect(tick('stop').phase).toBe('error');
  expect(h.move).not.toHaveBeenCalled();
});

it('returns by camera to the pose the mower last left from, then verifies contact without a map write', async () => {
  startDockReturn(id, 'mower'); tick(); await advance(100);
  expect(tick()).toMatchObject({ phase: 'done' });
  expect(h.move).toHaveBeenCalledWith('mower', expect.objectContaining({ reanchor: true }),
    { action: 'dock', distance: 0, fromDock: false, signature: 'frame', chargePose: LEFT_FROM }, expect.any(Function), expect.any(Function));
  expect(h.settle).toHaveBeenCalledWith('mower', true, expect.any(Function));
  expect(h.command).not.toHaveBeenCalled();
});

it('refuses when no departure pose was measured in the current map frame', async () => {
  rememberDockedPose('mower', LEFT_FROM, 1);
  startDockReturn(id, 'mower'); tick(); await advance(100);
  expect(tick('stop')).toMatchObject({ phase: 'error', error: expect.stringContaining('joystick') });
  expect(h.move).not.toHaveBeenCalled();
  expect(h.read).not.toHaveBeenCalled();
});

it('cancels an in-flight dock return before contact verification', async () => {
  let release!: () => void;
  h.move.mockImplementation((_sn, _operation, _input, _check, setMotion) => {
    setMotion('motion');
    return new Promise<void>(resolve => { release = resolve; });
  });
  startDockReturn(id, 'mower'); tick(); await advance(100);
  expect(tick('stop').phase).toBe('docking');
  expect(h.publish).toHaveBeenCalledWith('mower', { dock_measurement_control: { motion_id: 'motion', action: 'stop' } });
  release(); await advance(100);
  expect(tick('stop').phase).toBe('error');
  expect(h.settle).not.toHaveBeenCalled();
});
