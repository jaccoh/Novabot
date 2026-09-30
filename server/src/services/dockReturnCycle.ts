import { isDeviceOnline } from '../mqtt/broker.js';
import { publishToExtended } from '../mqtt/mapSync.js';
import { getFrameRevision, isMapInstallPending } from './frameValidation.js';
import { stablePosition } from './positionTelemetry.js';
import { readMowerMapSnapshot, withMowerMapOperation } from './mowerMapOperation.js';
import { frameSnapshotSignature } from './copyAlignment.js';
import { disarmEdgeWatch } from './scheduleRunner.js';
import { guardedDockMove, recallDockedPose, settleDockMotion } from './dockMotion.js';

type Phase = 'starting' | 'checking' | 'docking' | 'verifying' | 'done' | 'error';
export type DockReturnView = { cycleId: string; sn: string; phase: Phase; error?: string };
type Cycle = DockReturnView & { operatorAt: number; createdAt: number; revision: number; cancelled: boolean; finished: boolean; motionId?: string };
const cycles = new Map<string, Cycle>();
const sleep = (ms: number) => new Promise<void>(resolve => setTimeout(resolve, ms));
const view = (c: Cycle): DockReturnView => ({ cycleId: c.cycleId, sn: c.sn, phase: c.phase, ...(c.error ? { error: c.error } : {}) });
function stop(c: Cycle, reason: string): void {
  if (c.finished) return;
  c.cancelled = true;
  c.error ??= reason;
  if (c.motionId) publishToExtended(c.sn, { dock_measurement_control: { motion_id: c.motionId, action: 'stop' } });
}
function check(c: Cycle): void {
  if (c.cancelled) throw new Error(c.error ?? 'Dock return stopped.');
  if (performance.now() - c.operatorAt > 5_000 || performance.now() - c.createdAt > 120_000 ||
      !isDeviceOnline(c.sn) || getFrameRevision(c.sn) !== c.revision || isMapInstallPending(c.sn)) {
    stop(c, 'Dock return stopped: supervision, connection or map reference was lost.');
    throw new Error(c.error);
  }
}

/** Return only by the nearby camera marker and confirmed charging contact. No map or origin write. */
export function startDockReturn(cycleId: string, sn: string): DockReturnView {
  if (!/^[0-9a-f-]{36}$/i.test(cycleId) || !sn) throw new Error('Invalid dock return request.');
  for (const [id, c] of cycles) if (c.finished && performance.now() - c.createdAt > 30 * 60_000) cycles.delete(id);
  const existing = cycles.get(cycleId);
  if (existing) {
    if (existing.sn !== sn) throw new Error('Cycle belongs to another mower.');
    return view(existing);
  }
  if ([...cycles.values()].some(c => !c.finished && c.sn === sn)) throw new Error('A dock return is already running for this mower.');
  const c: Cycle = { cycleId, sn, phase: 'starting', operatorAt: -Infinity, createdAt: performance.now(),
    revision: getFrameRevision(sn), cancelled: false, finished: false };
  cycles.set(cycleId, c);
  void run(c);
  return view(c);
}

export function dockReturn(cycleId: string, sn: string, action: 'pulse' | 'stop'): DockReturnView {
  const c = cycles.get(cycleId);
  if (!c || c.sn !== sn) throw new Error('Unknown dock return cycle.');
  if (action === 'stop') stop(c, 'Dock return stopped by the operator.');
  else if (!c.finished && !c.cancelled) {
    if (!Number.isFinite(c.operatorAt) && performance.now() - c.createdAt >= 5_000) stop(c, 'Start confirmation expired.');
    if (Number.isFinite(c.operatorAt)) check(c);
    c.operatorAt = performance.now();
  }
  return view(c);
}

async function run(c: Cycle): Promise<void> {
  let timer: ReturnType<typeof setInterval> | undefined;
  try {
    while (!Number.isFinite(c.operatorAt) && !c.cancelled && performance.now() - c.createdAt < 5_000) await sleep(100);
    check(c);
    c.phase = 'checking';
    // The return is bounded by the pose the mower measured when it left this
    // dock in the current frame; saved map files may be shifted against it.
    const chargePose = recallDockedPose(c.sn, c.revision);
    if (!chargePose) throw new Error('No departure pose is known for this map frame. Return the mower to its dock with the joystick.');
    await withMowerMapOperation(c.sn, async operation => {
      operation.onManualControl = () => stop(c, 'Dock return stopped by manual control.');
      timer = setInterval(() => { try { check(c); } catch { /* cancellation is latched */ } }, 500);
      const before = await readMowerMapSnapshot(c.sn, operation);
      if (!before) throw new Error('Mower files could not be read.');
      const signature = frameSnapshotSignature(before);
      if (!stablePosition(c.sn)) throw new Error('Stop near the own dock and wait for stable RTK Fixed localization.');
      check(c);
      disarmEdgeWatch(c.sn, 'supervised camera dock return');
      c.phase = 'docking';
      await guardedDockMove(c.sn, operation, { action: 'dock', distance: 0, fromDock: false, signature, chargePose },
        () => check(c), id => { c.motionId = id; });
      check(c);
      c.phase = 'verifying';
      await settleDockMotion(c.sn, true, () => check(c));
      check(c);
      c.phase = 'done';
    }, true);
  } catch (error) {
    stop(c, error instanceof Error ? error.message : String(error));
    c.phase = 'error';
  } finally {
    clearInterval(timer);
    if (c.motionId) publishToExtended(c.sn, { dock_measurement_control: { motion_id: c.motionId, action: 'stop' } });
    c.finished = true;
  }
}
