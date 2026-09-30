import { randomUUID } from 'node:crypto';
import { publishToExtended } from '../mqtt/mapSync.js';
import type { MowerMapOperation } from './mowerMapOperation.js';
import { freshPositionState, stablePosition } from './positionTelemetry.js';

export class DockMotionError extends Error {}
/** A pose in the mower's current map frame (metres, radians). */
export type DockPose = { x: number; y: number; yaw: number };
/**
 * v4: a departure reports the pose the mower left from, and docking drives the
 * native visual action from a given charge pose. auto_recharge_server then
 * starts its camera approach directly (0.87..1.47 m in front, within 0.51 rad)
 * instead of the step-back search or nav2 (research/documents/auto-recharge-server-decompile.md).
 */
export const DOCK_MOTION_PROTOCOL = 'dock-measurement-motion-v4';

const sleep = (ms: number) => new Promise<void>(resolve => setTimeout(resolve, ms));
const isPose = (value: unknown): value is DockPose =>
  !!value && typeof value === 'object' && ['x', 'y', 'yaw'].every(k => Number.isFinite((value as Record<string, unknown>)[k]));

// The pose a mower last left its dock from, valid for one map frame revision.
// ponytail: in memory only; after a server restart the operator uses the joystick.
const departures = new Map<string, { pose: DockPose; revision: number }>();
export function rememberDockedPose(sn: string, pose: DockPose, revision: number): void { departures.set(sn, { pose, revision }); }
export function recallDockedPose(sn: string, revision: number): DockPose | undefined {
  const entry = departures.get(sn);
  return entry && entry.revision === revision ? entry.pose : undefined;
}

/** Shared by source measurement, own-dock reanchoring and camera return. Native watchdog owns the actual stop. */
export async function guardedDockMove(sn: string, operation: MowerMapOperation,
  input: { action: 'reverse' | 'dock'; distance: number; fromDock: boolean; signature: string; chargePose?: DockPose },
  check: () => void, setMotion: (id?: string) => void = () => {},
  beforeMove: () => void = () => {},
): Promise<{ startPose?: DockPose }> {
  if (input.action === 'dock' && !isPose(input.chargePose)) throw new Error('Docking needs the measured dock pose the mower left from.');
  check();
  const id = randomUUID();
  const control = (action: 'keepalive' | 'stop') => publishToExtended(sn, { dock_measurement_control: { motion_id: id, action } });
  setMotion(id);
  let stopped: unknown;
  const timer = setInterval(() => {
    try { if (stopped) return; check(); control('keepalive'); }
    catch (error) { stopped = error; control('stop'); }
  }, 500);
  try {
    const armed = await operation.command('dock_measurement_control', { action: 'arm', motion_id: id }, 5_000);
    check();
    if (stopped) throw stopped;
    if (armed?.result !== 0 || armed?.protocol !== DOCK_MOTION_PROTOCOL) throw new Error('The mower needs the current guarded dock recovery protocol. Update extended_commands.py first.');
    beforeMove();
    // A visual dock took 35 s live; the native lease allows 120 s.
    const result = await operation.command('dock_measurement_move', { action: input.action, distance_m: input.distance,
      from_dock: input.fromDock, recovery: operation.reanchor === true, motion_id: id, frame_fingerprint: input.signature,
      ...(input.chargePose ? { charge_pose: input.chargePose } : {}) }, input.action === 'dock' ? 100_000 : 60_000);
    if (result?.result !== 0 || result.protocol !== armed.protocol || result.frame_fingerprint !== input.signature ||
        (input.action === 'dock' && result.docked !== true)) {
      throw new DockMotionError(typeof result?.error === 'string' ? result.error : 'Movement stop was not confirmed. Check the mower and use its STOP button if needed.');
    }
    if (input.fromDock && !isPose(result.start_pose)) throw new DockMotionError('The mower did not report the pose it left the dock from.');
    if (stopped) throw stopped;
    check();
    return input.fromDock ? { startPose: result.start_pose as DockPose } : {};
  } finally { clearInterval(timer); control('stop'); setMotion(); }
}

/** All samples must follow the completed movement; cached dock poses cannot satisfy this. */
export async function settleDockMotion(sn: string, docked: boolean, check: () => void) {
  const after = Date.now(), deadline = performance.now() + 25_000;
  while (performance.now() < deadline) {
    check();
    const sample = stablePosition(sn, { after, docked });
    if (sample && (docked || !freshPositionState(sn).docked)) return sample;
    await sleep(250);
  }
  throw new Error('The mower did not settle with fresh RTK Fixed. Return it to its own dock with the joystick before retrying.');
}
