import { afterEach, beforeEach, expect, it, vi } from 'vitest';

const state = vi.hoisted(() => ({ online: true, stable: true, leaseValid: true, unvalidated: new Set<string>(), revisions: new Map<string, number>(), command: vi.fn() }));
vi.mock('../../mqtt/broker.js', () => ({ isDeviceOnline: () => state.online }));
vi.mock('../../services/mowerFileCapability.js', () => ({ isOpenNovaMower: () => true }));
vi.mock('../../services/frameValidation.js', () => ({ isFrameUnvalidated: (sn: string) => state.unvalidated.has(sn), getFrameRevision: (sn: string) => state.revisions.get(sn) ?? 0 }));
vi.mock('../../services/positionTelemetry.js', () => ({ stablePosition: () => state.stable ? { x: 0, y: 0 } : null }));
vi.mock('../../services/mowerMapOperation.js', () => ({
  withMowerMapOperation: vi.fn(async (sn, run) => run({ sn, id: 'alignment-test', command: (...args: unknown[]) => state.command(sn, ...args) })),
  assertMowerMapOperation: vi.fn((sn, operation) => { if (!state.leaseValid || operation?.sn !== sn) throw new Error('Map operation lease is no longer active'); }),
  readMowerMapSnapshot: vi.fn(),
}));

import { readMowerMapSnapshot, type MowerMapOperation } from '../../services/mowerMapOperation.js';
import { beginCopyAlignment, captureCopyAlignment, consumeCopyAlignment, frameSnapshotSignature, getCopyAlignment, validateCopyAlignment } from '../../services/copyAlignment.js';

const sourceSn = 'LFIN_ALIGNMENT_SOURCE', targetSn = 'LFIN_ALIGNMENT_TARGET', canonical = 'map0';
const snapshot = () => {
  const csv_files = {
    'map_info.json': JSON.stringify({ charging_pose: { x: .03, y: .73, orientation: -1.5 } }),
    'map0_work.csv': '0,0\n3,0\n3,3\n0,3\n',
    'map0_0_obstacle.csv': '1,1\n2,1\n2,2\n1,2\n',
  };
  return {
  result: 0, snapshot_consistent: true,
  pos_json: JSON.stringify({ time_stamp: 123, utm_origin: { x: 300000, y: 5700000, z: 0, utm_zone: 32 } }),
  charging_station_yaml: 'charging_pose: [0.03,0.73,-1.5]',
  csv_files, x3_csv_files: { ...csv_files },
  };
};
let snapshots: Record<string, ReturnType<typeof snapshot>>;
let counts: Record<string, number>;
let override: (raw: Record<string, unknown>) => Record<string, unknown>;
let elapsedMs: number;
let clockOffsets: Record<string, number>;
const advance = (ms: number) => { elapsedMs += ms; vi.setSystemTime(Date.now() + ms); };

beforeEach(() => {
  vi.useFakeTimers(); vi.setSystemTime(new Date('2026-09-26T10:00:00Z'));
  elapsedMs = 0; clockOffsets = {};
  vi.spyOn(performance, 'now').mockImplementation(() => elapsedMs);
  state.online = true; state.stable = true; state.leaseValid = true; state.unvalidated.clear(); state.revisions.clear(); state.command.mockReset();
  counts = {}; override = raw => raw;
  snapshots = { [sourceSn]: snapshot(), [targetSn]: snapshot() };
  vi.mocked(readMowerMapSnapshot).mockReset().mockImplementation(async sn => structuredClone(snapshots[sn]));
  state.command.mockImplementation(async (sn: string, cmd: string, _params: unknown, timeout: number) => {
    expect(['measure_dock_marker', 'measure_runtime_frame']).toContain(cmd);
    expect(timeout).toBe(cmd === 'measure_dock_marker' ? 50_000 : 25_000);
    const i = counts[sn] ?? 0;
    if (cmd === 'measure_dock_marker') counts[sn] = i + 1;
    const x = sn === sourceSn ? 2 : 12, y = sn === sourceSn ? 1 : 21;
    const started = Date.now() / 1000 + (clockOffsets[sn] ?? 0) + .1;
    advance(8_000);
    const runtime_frame = { x: 0, y: 0, spread_m: .005, sample_count: 30, unique_stamps: 30,
      max_pair_dt_s: 0, capture_started: started, capture_finished: started + 7 };
    if (cmd === 'measure_runtime_frame') return override({ result: 0, protocol: 'runtime-map-frame-v1', runtime_frame,
      frame_fingerprint: frameSnapshotSignature(snapshots[sn]) });
    return override({ result: 0, protocol: 'aruco-map-marker-v1', runtime_frame,
      marker: { x, y, z: .2, yaw: .7 }, base: { x: x - 1 + i * .2, y, z: 0, yaw: 0 },
      sample_count: 22, unique_stamps: 22, spread_m: .01, yaw_spread_rad: .018, max_pair_dt_s: .11,
      frame_fingerprint: frameSnapshotSignature(snapshots[sn]), capture_started: started, capture_finished: started + 7,
    });
  });
});
afterEach(() => { vi.restoreAllMocks(); vi.useRealTimers(); });

async function complete() {
  const s = await beginCopyAlignment(targetSn, sourceSn, canonical);
  const phases = ['source_first'];
  for (const side of ['source', 'source', 'target', 'target'] as const) phases.push((await captureCopyAlignment(s.alignmentId, side)).phase);
  expect(phases).toEqual(['source_first', 'source_second', 'target_first', 'target_second', 'ready']);
  return s.alignmentId;
}
const operation = (sn: string): MowerMapOperation => ({ sn, id: sn, command: (...args) => state.command(sn, ...args) });
const validate = (id: string) => validateCopyAlignment(id, { targetSn, sourceSn, canonical, sourceSnapshot: snapshots[sourceSn], targetSnapshot: snapshots[targetSn],
  sourceOperation: operation(sourceSn), targetOperation: operation(targetSn) });

it('has the same normalized fingerprint as Python BE doubles and ignores only origin timestamp/JSON formatting', () => {
  const s = snapshot();
  expect(frameSnapshotSignature(s)).toBe('b29d26ce4347bc20909832fb94d8eea345cd844108ab7d502111970607c8a3e4');
  s.pos_json = '{"utm_origin":{"utm_zone":32,"z":0,"y":5700000,"x":300000},"time_stamp":456}';
  expect(frameSnapshotSignature(s)).toBe(frameSnapshotSignature(snapshot()));
  s.charging_station_yaml = 'charging_pose: [0,0,0]';
  expect(() => frameSnapshotSignature(s)).toThrow('not confirmed');
});

it.each([[31.7, 4.3], [-35, 10], [60, -60]])('accepts independent mower clock offsets of %ss and %ss through post-install verification', async (sourceOffset, targetOffset) => {
  clockOffsets = { [sourceSn]: sourceOffset, [targetSn]: targetOffset };
  const id = await complete();
  const checked = await validate(id);
  expect(checked.dockAtB).toEqual({ x: 10.03, y: 20.73 });
  state.unvalidated.add(targetSn); state.revisions.set(targetSn, 1);
  await expect(checked.verifyRuntime()).resolves.toEqual(snapshots[targetSn]);
});

it('requires all four fresh viewpoints, translates the saved source dock rather than the marker, and does not mutate snapshots', async () => {
  const before = structuredClone(snapshots);
  const id = await complete();
  const result = await validate(id);
  expect(result.dockAtB.x).toBeCloseTo(10.03); expect(result.dockAtB.y).toBeCloseTo(20.73);
  expect(result.captures.source).toHaveLength(2); expect(result.captures.target).toHaveLength(2);
  result.captures.source[0].marker.x = 999;
  expect((await validate(id)).dockAtB.x).toBeCloseTo(10.03);
  expect(snapshots).toEqual(before);
  expect(state.command).toHaveBeenCalledTimes(8);
  consumeCopyAlignment(id);
  await expect(validate(id)).rejects.toThrow('unknown');
});

it('rejects unknown/expired sessions, different source/target/zone and reordered or incomplete captures', async () => {
  expect(() => getCopyAlignment('missing', targetSn, sourceSn, canonical)).toThrow('unknown');
  const s = await beginCopyAlignment(targetSn, sourceSn, canonical);
  expect(() => getCopyAlignment(s.alignmentId, sourceSn, targetSn, canonical)).toThrow('different');
  expect(() => getCopyAlignment(s.alignmentId, targetSn, sourceSn, 'map1')).toThrow('different');
  await expect(captureCopyAlignment(s.alignmentId, 'target')).rejects.toThrow('order');
  await expect(validate(s.alignmentId)).rejects.toThrow('twice');
  vi.setSystemTime(Date.now() + 20 * 60_000);
  await expect(captureCopyAlignment(s.alignmentId, 'source')).rejects.toThrow('expired');
});

it('rejects invalid direct inputs and different UTM zones before marker measurements', async () => {
  await expect(beginCopyAlignment(targetSn, sourceSn, 0 as unknown as string)).rejects.toThrow('valid source zone');
  await expect(beginCopyAlignment(targetSn, {} as string, canonical)).rejects.toThrow('different source');
  snapshots[targetSn].pos_json = snapshots[targetSn].pos_json.replace('"utm_zone":32', '"utm_zone":31');
  await expect(beginCopyAlignment(targetSn, sourceSn, canonical)).rejects.toThrow('different UTM zones');
  expect(state.command).not.toHaveBeenCalled();
});

it('never lets concurrent captures advance the same wizard step twice', async () => {
  const s = await beginCopyAlignment(targetSn, sourceSn, canonical);
  const original = state.command.getMockImplementation()!;
  let release: () => void = () => {};
  const bothStarted = new Promise<void>(resolve => { release = resolve; });
  let started = 0;
  // The real operation lease rejects this race sooner; retain the final state check as well.
  state.command.mockImplementation(async (...args: unknown[]) => {
    const raw = await original(...args);
    if (++started === 2) release();
    await bothStarted;
    return raw;
  });
  const results = await Promise.allSettled([captureCopyAlignment(s.alignmentId, 'source'), captureCopyAlignment(s.alignmentId, 'source')]);
  expect(results.filter(r => r.status === 'fulfilled')).toHaveLength(1);
  expect(results.find(r => r.status === 'rejected')).toMatchObject({ reason: expect.objectContaining({ message: expect.stringContaining('Another capture') }) });
  expect(getCopyAlignment(s.alignmentId, targetSn, sourceSn, canonical).captures.source).toHaveLength(1);
});

it.each([
  ['missing protocol', (r: Record<string, unknown>) => ({ ...r, protocol: undefined }), 'verified'],
  ['old protocol', (r: Record<string, unknown>) => ({ ...r, protocol: 'legacy-pose' }), 'verified'],
  ['device error', (r: Record<string, unknown>) => ({ ...r, result: 1, error: 'fresh idle robot required' }), 'fresh idle robot required'],
  ['error result', (r: Record<string, unknown>) => ({ ...r, result: 1 }), 'verified'],
  ['wrong frame', (r: Record<string, unknown>) => ({ ...r, frame_fingerprint: 'other' }), 'different mower frame'],
  ['runtime after marker window', (r: Record<string, unknown>) => ({ ...r, capture_started: Number(r.capture_started) - 30, capture_finished: Number(r.capture_finished) - 30 }), 'stale'],
  ['runtime before marker window', (r: Record<string, unknown>) => ({ ...r, capture_started: Number(r.capture_started) + 30, capture_finished: Number(r.capture_finished) + 30 }), 'stale'],
  ['repeated frames', (r: Record<string, unknown>) => ({ ...r, unique_stamps: 1 }), 'stable'],
  ['too brief', (r: Record<string, unknown>) => ({ ...r, capture_finished: Number(r.capture_started) + 1 }), 'stale'],
  ['window longer than request', (r: Record<string, unknown>) => ({ ...r, capture_finished: Number(r.capture_started) + 12 }), 'stale'],
  ['position spread', (r: Record<string, unknown>) => ({ ...r, spread_m: .031 }), 'stable'],
  ['yaw spread', (r: Record<string, unknown>) => ({ ...r, yaw_spread_rad: .04 }), 'stable'],
  ['unsynchronized pose', (r: Record<string, unknown>) => ({ ...r, max_pair_dt_s: .13 }), 'synchronized'],
  ['missing marker', (r: Record<string, unknown>) => ({ ...r, marker: { x: 1 } }), 'incomplete'],
  ['missing runtime frame', (r: Record<string, unknown>) => ({ ...r, runtime_frame: undefined }), 'runtime frame measurement is incomplete'],
  ['few runtime pairs', (r: Record<string, unknown>) => ({ ...r, runtime_frame: { ...(r.runtime_frame as object), unique_stamps: 19 } }), 'exactly synchronized'],
  ['approximate runtime pairs', (r: Record<string, unknown>) => ({ ...r, runtime_frame: { ...(r.runtime_frame as object), max_pair_dt_s: .00001 } }), 'exactly synchronized'],
  ['runtime spread', (r: Record<string, unknown>) => ({ ...r, runtime_frame: { ...(r.runtime_frame as object), spread_m: .021 } }), 'exactly synchronized'],
  ['too far away', (r: Record<string, unknown>) => ({ ...r, base: { x: 0, y: 0, z: 0, yaw: 0 } }), 'closer'],
])('rejects %s without advancing the wizard', async (_name, change, message) => {
  const s = await beginCopyAlignment(targetSn, sourceSn, canonical);
  override = change;
  await expect(captureCopyAlignment(s.alignmentId, 'source')).rejects.toThrow(message);
  expect(getCopyAlignment(s.alignmentId, targetSn, sourceSn, canonical).phase).toBe('source_first');
});

it.each([0, -30])('rejects repeated or backward marker windows (%ss) even with a newly correlated command', async offset => {
  clockOffsets[sourceSn] = 31.7;
  const s = await beginCopyAlignment(targetSn, sourceSn, canonical);
  const first = (await captureCopyAlignment(s.alignmentId, 'source')).captures.source[0];
  override = raw => ({ ...raw, capture_started: first.capture_started + offset, capture_finished: first.capture_finished + offset,
    runtime_frame: { ...(raw.runtime_frame as object), capture_started: first.capture_started + offset, capture_finished: first.capture_finished + offset } });
  await expect(captureCopyAlignment(s.alignmentId, 'source')).rejects.toThrow('repeats an earlier');
  expect(getCopyAlignment(s.alignmentId, targetSn, sourceSn, canonical).phase).toBe('source_second');
});

it.each(['measure_dock_marker', 'measure_runtime_frame'])('rejects a delayed %s response, not just time spent after receiving it', async delayedCommand => {
  const id = delayedCommand === 'measure_dock_marker'
    ? (await beginCopyAlignment(targetSn, sourceSn, canonical)).alignmentId : await complete();
  const original = state.command.getMockImplementation()!;
  state.command.mockImplementation(async (sn: string, cmd: string, ...args: unknown[]) => {
    const raw = await original(sn, cmd, ...args);
    if (cmd === delayedCommand && sn === sourceSn) advance(11_000);
    return raw;
  });
  await expect(delayedCommand === 'measure_dock_marker' ? captureCopyAlignment(id, 'source') : validate(id)).rejects.toThrow('measurement is stale');
});

it('uses monotonic command time if the server wall clock changes while receiving a response', async () => {
  const s = await beginCopyAlignment(targetSn, sourceSn, canonical);
  const original = state.command.getMockImplementation()!;
  state.command.mockImplementation(async (...args: unknown[]) => {
    const raw = await original(...args);
    vi.setSystemTime(Date.now() - 60_000);
    return raw;
  });
  await expect(captureCopyAlignment(s.alignmentId, 'source')).resolves.toHaveProperty('phase', 'source_second');
});

it('rejects a marker observation that ages while reading its final native snapshot', async () => {
  const s = await beginCopyAlignment(targetSn, sourceSn, canonical);
  let reads = 0;
  vi.mocked(readMowerMapSnapshot).mockImplementation(async sn => {
    if (++reads === 2) advance(10_000);
    return structuredClone(snapshots[sn]);
  });
  await expect(captureCopyAlignment(s.alignmentId, 'source')).rejects.toThrow('marker measurement is stale');
});

it('requires a distinct second viewpoint and checks independent marker position and circular heading agreement', async () => {
  const s = await beginCopyAlignment(targetSn, sourceSn, canonical);
  await captureCopyAlignment(s.alignmentId, 'source');
  counts[sourceSn] = 0;
  await expect(captureCopyAlignment(s.alignmentId, 'source')).rejects.toThrow('15 cm');
  counts[sourceSn] = 1;
  override = raw => ({ ...raw, marker: { ...(raw.marker as object), x: 2.04 } });
  await expect(captureCopyAlignment(s.alignmentId, 'source')).rejects.toThrow('disagree');
  override = raw => raw;
  await captureCopyAlignment(s.alignmentId, 'source');
  override = raw => ({ ...raw, marker: { ...(raw.marker as object), yaw: .73 } });
  await expect(captureCopyAlignment(s.alignmentId, 'target')).rejects.toThrow('headings');
});

it('binds final validation to both native frames and the complete source work/obstacle geometry', async () => {
  const id = await complete();
  const source = structuredClone(snapshots[sourceSn]), target = structuredClone(snapshots[targetSn]);
  snapshots[targetSn].pos_json = snapshots[targetSn].pos_json.replace('300000', '300001');
  await expect(validate(id)).rejects.toThrow('changed');
  snapshots[targetSn] = target;
  snapshots[sourceSn].csv_files['map0_0_obstacle.csv'] += '1.1,1.1\n';
  await expect(validate(id)).rejects.toThrow('changed');
  snapshots[sourceSn] = source;
  expect((await validate(id)).phase).toBe('ready');
  state.revisions.set(sourceSn, 1);
  await expect(validate(id)).rejects.toThrow('frame changed');
});

it('rejects frame changes during capture, degraded localization and offline mowers', async () => {
  const s = await beginCopyAlignment(targetSn, sourceSn, canonical);
  let reads = 0;
  vi.mocked(readMowerMapSnapshot).mockImplementation(async sn => {
    const result = structuredClone(snapshots[sn]);
    if (++reads === 2) result.pos_json = result.pos_json.replace('300000', '300001');
    return result;
  });
  await expect(captureCopyAlignment(s.alignmentId, 'source')).rejects.toThrow('changed');
  state.stable = false;
  await expect(captureCopyAlignment(s.alignmentId, 'source')).rejects.toThrow('stable RTK');
  state.stable = true; state.unvalidated.add(sourceSn);
  expect(() => getCopyAlignment(s.alignmentId, targetSn, sourceSn, canonical)).toThrow('validated');
  state.unvalidated.clear(); state.online = false;
  await expect(beginCopyAlignment(targetSn, sourceSn, canonical)).rejects.toThrow('online');
});

it('invalidates a registration when the runtime offset changes despite matching files and marker poses', async () => {
  const s = await beginCopyAlignment(targetSn, sourceSn, canonical);
  await captureCopyAlignment(s.alignmentId, 'source');
  override = raw => ({ ...raw, runtime_frame: { ...(raw.runtime_frame as object), x: .13 } });
  await expect(captureCopyAlignment(s.alignmentId, 'source')).rejects.toThrow('runtime mower frame changed');
  expect(() => getCopyAlignment(s.alignmentId, targetSn, sourceSn, canonical)).toThrow('unknown');
});

it.each([[.071, 0], [.022, 0], [0, -.020001], [.0142, .0142]])('rejects stable temporary compensation (%s,%s) at the first capture', async (x, y) => {
  const s = await beginCopyAlignment(targetSn, sourceSn, canonical);
  override = raw => ({ ...raw, runtime_frame: { ...(raw.runtime_frame as object), x, y } });
  await expect(captureCopyAlignment(s.alignmentId, 'source')).rejects.toThrow('Let localization settle');
  expect(getCopyAlignment(s.alignmentId, targetSn, sourceSn, canonical).phase).toBe('source_first');
});

it.each([[.02, 0], [-.02, 0], [.014, -.014]])('accepts near-zero boundary (%s,%s) without normalizing the measured offset', async (x, y) => {
  const s = await beginCopyAlignment(targetSn, sourceSn, canonical);
  override = raw => ({ ...raw, runtime_frame: { ...(raw.runtime_frame as object), x, y } });
  const result = await captureCopyAlignment(s.alignmentId, 'source');
  expect(result.captures.source[0].runtime_frame).toMatchObject({ x, y });
});

it('invalidates opposite near-zero offsets when their change exceeds 2cm', async () => {
  const s = await beginCopyAlignment(targetSn, sourceSn, canonical);
  override = raw => ({ ...raw, runtime_frame: { ...(raw.runtime_frame as object), x: -.015 } });
  await captureCopyAlignment(s.alignmentId, 'source');
  override = raw => ({ ...raw, runtime_frame: { ...(raw.runtime_frame as object), x: .015 } });
  await expect(captureCopyAlignment(s.alignmentId, 'source')).rejects.toThrow('runtime mower frame changed');
  expect(() => getCopyAlignment(s.alignmentId, targetSn, sourceSn, canonical)).toThrow('unknown');
});

it.each([sourceSn, targetSn])('checks fresh runtime against each original capture of %s, not just their average', async changedSn => {
  const s = await beginCopyAlignment(targetSn, sourceSn, canonical);
  for (const side of ['source', 'source', 'target', 'target'] as const) {
    const sn = side === 'source' ? sourceSn : targetSn;
    const x = sn === changedSn && counts[sn] === 1 ? .018 : 0;
    override = raw => ({ ...raw, runtime_frame: { ...(raw.runtime_frame as object), x } });
    await captureCopyAlignment(s.alignmentId, side);
  }
  override = raw => raw;
  const original = state.command.getMockImplementation()!;
  state.command.mockImplementation(async (sn: string, ...args: unknown[]) => {
    const raw = await original(sn, ...args);
    return sn === changedSn ? { ...raw, runtime_frame: { ...raw.runtime_frame, x: .026 } } : raw;
  });
  await expect(validate(s.alignmentId)).rejects.toThrow('runtime mower frame changed');
  expect(() => getCopyAlignment(s.alignmentId, targetSn, sourceSn, canonical)).toThrow('unknown');
});

it.each([
  ['legacy firmware', (raw: Record<string, unknown>) => ({ ...raw, protocol: undefined }), 'verified runtime'],
  ['failure', (raw: Record<string, unknown>) => ({ ...raw, result: 1, error: 'moving' }), 'moving'],
  ['wrong frame', (raw: Record<string, unknown>) => ({ ...raw, frame_fingerprint: 'other' }), 'different mower frame'],
  ['cached runtime', (raw: Record<string, unknown>) => ({ ...raw, runtime_frame: { ...(raw.runtime_frame as object), capture_started: 0, capture_finished: 7 } }), 'stale'],
])('requires a fresh successful runtime command: %s', async (_name, change, message) => {
  const id = await complete();
  override = change;
  await expect(validate(id)).rejects.toThrow(message);
});

it('re-reads source geometry after measurements instead of accepting the earlier snapshot', async () => {
  const id = await complete();
  const original = state.command.getMockImplementation()!;
  state.command.mockImplementation(async (...args: unknown[]) => {
    const raw = await original(...args);
    snapshots[sourceSn].csv_files['map0_0_obstacle.csv'] += '1.1,1.1\n';
    return raw;
  });
  await expect(validate(id)).rejects.toThrow('source zone or mower frame changed');
});

it.each(['csv_files', 'x3_csv_files', 'pos_json', 'charging_station_yaml'])('refuses to overwrite target %s changed during preflight', async key => {
  const id = await complete();
  const original = state.command.getMockImplementation()!;
  state.command.mockImplementation(async (sn: string, ...args: unknown[]) => {
    const raw = await original(sn, ...args);
    if (sn === targetSn) {
      if (key === 'pos_json') snapshots[sn].pos_json += ' ';
      else if (key === 'charging_station_yaml') snapshots[sn].charging_station_yaml += '\n# changed';
      else (snapshots[sn][key as 'csv_files' | 'x3_csv_files'] as Record<string, string>)['map1_work.csv'] = '10,10\n12,10\n12,12\n';
    }
    return raw;
  });
  await expect(validate(id)).rejects.toThrow('target map files changed during preflight');
});

it('compares target file mappings by keys and bytes, independent of key insertion order', async () => {
  const id = await complete();
  const original = state.command.getMockImplementation()!;
  state.command.mockImplementation(async (sn: string, ...args: unknown[]) => {
    const raw = await original(sn, ...args);
    if (sn === targetSn) for (const key of ['csv_files', 'x3_csv_files'] as const) {
      snapshots[sn][key] = Object.fromEntries(Object.entries(snapshots[sn][key]).reverse()) as typeof snapshots[string][typeof key];
    }
    return raw;
  });
  await expect(validate(id)).resolves.toHaveProperty('phase', 'ready');
});

it('rechecks freshness after waiting for both native file reads', async () => {
  const id = await complete();
  vi.mocked(readMowerMapSnapshot).mockImplementation(async sn => {
    advance(15_000);
    return structuredClone(snapshots[sn]);
  });
  await expect(validate(id)).rejects.toThrow('runtime frame measurement is stale');
});

it.each([sourceSn, targetSn])('rejects reusing the preflight runtime window for %s after installation', async replaySn => {
  const id = await complete();
  const original = state.command.getMockImplementation()!;
  let saved: Record<string, unknown>;
  state.command.mockImplementation(async (sn: string, ...args: unknown[]) => {
    const raw = await original(sn, ...args);
    if (sn === replaySn) saved = structuredClone(raw.runtime_frame);
    return raw;
  });
  const checked = await validate(id);
  state.unvalidated.add(targetSn); state.revisions.set(targetSn, 1);
  state.command.mockImplementation(async (sn: string, ...args: unknown[]) => {
    const raw = await original(sn, ...args);
    return sn === replaySn ? { ...raw, runtime_frame: saved } : raw;
  });
  await expect(checked.verifyRuntime()).rejects.toThrow('repeats an earlier');
  expect(state.unvalidated.has(targetSn)).toBe(true);
});

it('does not release its caller while a second runtime command is still pending after the first fails', async () => {
  const id = await complete();
  const original = state.command.getMockImplementation()!;
  let release: () => void = () => {};
  const held = new Promise<void>(resolve => { release = resolve; });
  state.command.mockImplementation(async (sn: string, ...args: unknown[]) => {
    if (sn === sourceSn) throw new Error('source transport failed');
    await held;
    return original(sn, ...args);
  });
  let settled = false;
  const pending = validate(id).finally(() => { settled = true; });
  const assertion = expect(pending).rejects.toThrow('source transport failed');
  for (let i = 0; i < 5; i++) await Promise.resolve();
  expect(settled).toBe(false);
  release();
  await assertion;
});

it('verifies runtime after exactly one guarded target installation and accepts added target geometry', async () => {
  const id = await complete();
  const checked = await validate(id);
  state.unvalidated.add(targetSn); state.revisions.set(targetSn, 1);
  (snapshots[targetSn].csv_files as Record<string, string>)['map1_work.csv'] = '10,10\n11,10\n11,11\n';
  expect(await checked.verifyRuntime()).toEqual(snapshots[targetSn]);
  // Validation does not perform the caller's commit, consume, or frame release.
  expect(state.unvalidated.has(targetSn)).toBe(true);
  expect(state.command.mock.calls.filter(([, command]) => command === 'measure_runtime_frame')).toHaveLength(4);
});

it.each(['no install', 'second revision', 'source revision', 'released target', 'lost lease', 'expired', 'consumed'])('rejects post-install verification with %s', async change => {
  const id = await complete();
  const checked = await validate(id);
  state.unvalidated.add(targetSn); state.revisions.set(targetSn, 1);
  if (change === 'no install') state.revisions.set(targetSn, 0);
  if (change === 'second revision') state.revisions.set(targetSn, 2);
  if (change === 'source revision') state.revisions.set(sourceSn, 1);
  if (change === 'released target') state.unvalidated.delete(targetSn);
  if (change === 'lost lease') state.leaseValid = false;
  if (change === 'expired') vi.setSystemTime(Date.now() + 20 * 60_000);
  if (change === 'consumed') consumeCopyAlignment(id);
  const before = state.command.mock.calls.length;
  await expect(checked.verifyRuntime()).rejects.toThrow();
  expect(state.command).toHaveBeenCalledTimes(before);
});

it.each([sourceSn, targetSn])('rejects post-install runtime drift on %s and keeps the target blocked', async changedSn => {
  const id = await complete();
  const checked = await validate(id);
  state.unvalidated.add(targetSn); state.revisions.set(targetSn, 1);
  const original = state.command.getMockImplementation()!;
  state.command.mockImplementation(async (sn: string, ...args: unknown[]) => {
    const raw = await original(sn, ...args);
    return sn === changedSn ? { ...raw, runtime_frame: { ...raw.runtime_frame, y: -.274 } } : raw;
  });
  await expect(checked.verifyRuntime()).rejects.toThrow('runtime mower frame changed');
  expect(state.unvalidated.has(targetSn)).toBe(true);
  expect(() => getCopyAlignment(id, targetSn, sourceSn, canonical)).toThrow('unknown');
});

it('rejects session expiry while fresh runtime measurements are in flight', async () => {
  const id = await complete();
  const original = state.command.getMockImplementation()!;
  state.command.mockImplementation(async (...args: unknown[]) => {
    const raw = await original(...args);
    vi.setSystemTime(Date.now() + 20 * 60_000);
    return raw;
  });
  await expect(validate(id)).rejects.toThrow('expired');
});
