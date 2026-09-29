import { randomUUID } from 'node:crypto';
import { publishToExtended } from '../mqtt/mapSync.js';
import type { MowerMapOperation } from './mowerMapOperation.js';
import { freshPositionState, stablePosition } from './positionTelemetry.js';

export class DockMotionError extends Error {}

const sleep = (ms: number) => new Promise<void>(resolve => setTimeout(resolve, ms));

/** Shared by source measurement and own-dock recovery. Native watchdog owns the actual stop. */
export async function guardedDockMove(sn: string, operation: MowerMapOperation,
  input: { action: 'reverse' | 'dock'; distance: number; fromDock: boolean; signature: string },
  check: () => void, setMotion: (id?: string) => void = () => {},
  beforeMove: () => void = () => {},
): Promise<void> {
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
    const supported = operation.reanchor ? armed?.protocol === 'dock-measurement-motion-v3' : ['dock-measurement-motion-v2', 'dock-measurement-motion-v3'].includes(String(armed?.protocol));
    if (armed?.result !== 0 || !supported) throw new Error('The mower needs the current guarded dock recovery protocol. Update extended_commands.py first.');
    beforeMove();
    const result = await operation.command('dock_measurement_move', { action: input.action, distance_m: input.distance,
      from_dock: input.fromDock, recovery: operation.reanchor === true, motion_id: id, frame_fingerprint: input.signature }, 60_000);
    if (result?.result !== 0 || result.protocol !== armed.protocol || result.frame_fingerprint !== input.signature ||
        (input.action === 'dock' && result.docked !== true)) {
      throw new DockMotionError(typeof result?.error === 'string' ? result.error : 'Movement stop was not confirmed. Check the mower and use its STOP button if needed.');
    }
    if (stopped) throw stopped;
    check();
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
