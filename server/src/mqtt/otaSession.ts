/**
 * OTA session phases (issue #130).
 *
 * Between the mower reporting 100% and the actual reboot there can be a
 * minute of silence (run_ota.sh copies the tree, stops the node stack). The
 * broker sees the disconnect / reconnect and the first report_state carries
 * the new sw_version, so the server is the one place that can string the
 * whole update together. Clients only render `phase` + `since`.
 *
 * ponytail: in-memory only; a container restart mid-OTA drops the session.
 */
import { emitOtaEvent } from '../dashboard/socketHandler.js';

export type OtaPhase =
  | 'downloading' | 'unpacking' | 'installing'
  | 'awaiting-reboot' | 'rebooting' | 'back'
  | 'done' | 'rolled-back' | 'failed' | 'stalled';

export interface OtaSession {
  sn: string;
  phase: OtaPhase;
  since: number;
  startedAt: number;
  target: string;
  from: string | null;
  /** Version reported after the reboot (done / rolled-back). */
  reported?: string;
  lastState?: unknown;
}

const AWAITING_REBOOT_TIMEOUT_MS = 5 * 60_000;
/** Silence while downloading/unpacking/installing; every progress message restarts it. */
const PROGRESS_SILENCE_MS = 10 * 60_000;
const FINISHED_TTL_MS = 10 * 60_000;
const TERMINAL: ReadonlySet<OtaPhase> = new Set(['done', 'rolled-back', 'failed', 'stalled']);
const IN_PROGRESS: ReadonlySet<OtaPhase> = new Set(['downloading', 'unpacking', 'installing']);

const sessions = new Map<string, OtaSession>();
const timers = new Map<string, NodeJS.Timeout>();

const norm = (v: string | null | undefined) => String(v ?? '').replace(/^v+/i, '').trim();

function armTimer(sn: string, phase: OtaPhase): void {
  clearTimeout(timers.get(sn));
  timers.delete(sn);
  if (phase === 'awaiting-reboot') {
    timers.set(sn, setTimeout(() => setPhase(sn, 'stalled'), AWAITING_REBOOT_TIMEOUT_MS));
  } else if (IN_PROGRESS.has(phase)) {
    timers.set(sn, setTimeout(() => setPhase(sn, 'stalled'), PROGRESS_SILENCE_MS));
  } else if (TERMINAL.has(phase)) {
    timers.set(sn, setTimeout(() => { sessions.delete(sn); timers.delete(sn); }, FINISHED_TTL_MS));
  }
}

function setPhase(sn: string, phase: OtaPhase): void {
  const s = sessions.get(sn);
  if (!s || s.phase === phase) return;
  s.phase = phase;
  s.since = Date.now();
  emitOtaEvent(sn, 'phase', { ...s });
  armTimer(sn, phase);
}

export function otaSessionStarted(sn: string, target: string, from: string | null): void {
  clearTimeout(timers.get(sn));
  timers.delete(sn);
  const now = Date.now();
  sessions.set(sn, { sn, phase: 'downloading', since: now, startedAt: now, target, from });
  emitOtaEvent(sn, 'phase', { ...sessions.get(sn)! });
  armTimer(sn, 'downloading');
}

/** Raw ota_upgrade_state from the device (status + percentage). */
export function otaSessionState(sn: string, state: { status?: unknown; percentage?: unknown; progress?: unknown }): void {
  const s = sessions.get(sn);
  if (!s || TERMINAL.has(s.phase)) return;
  s.lastState = state;
  armTimer(sn, s.phase); // any message proves the update is alive
  const status = String(state.status ?? '');
  if (status === 'success') return setPhase(sn, 'awaiting-reboot');
  if (status === 'failed' || status === 'error') return setPhase(sn, 'failed');
  const raw = Number(state.percentage ?? state.progress);
  if (!isFinite(raw)) return;
  const pct = raw <= 1 ? raw * 100 : raw;
  // ponytail: 62/68 boundaries come from the mower's run_ota.sh (see OTA.md)
  setPhase(sn, pct < 62 ? 'downloading' : pct < 68 ? 'unpacking' : 'installing');
}

export function otaSessionDisconnect(sn: string): void {
  const s = sessions.get(sn);
  if (s && (s.phase === 'awaiting-reboot' || s.phase === 'installing' || s.phase === 'stalled')) setPhase(sn, 'rebooting');
}

export function otaSessionConnect(sn: string): void {
  if (sessions.get(sn)?.phase === 'rebooting') setPhase(sn, 'back');
}

/** sw_version from every report_state. Decides done/rolled-back once the device is back. */
export function otaSessionVersion(sn: string, version: string): void {
  const s = sessions.get(sn);
  if (!s || s.phase === 'done' || s.phase === 'rolled-back' || s.phase === 'failed') return;
  if (s.phase === 'back') {
    s.reported = version;
    setPhase(sn, norm(version) === norm(s.target) ? 'done' : 'rolled-back');
    return;
  }
  // The new build itself proves the install, also when progress or the
  // reboot was missed (field report: modal stuck on "downloading").
  if (norm(version) === norm(s.target) && norm(version) !== norm(s.from)) {
    s.reported = version;
    setPhase(sn, 'done');
  }
}

export function getOtaSession(sn: string): OtaSession | undefined {
  return sessions.get(sn);
}

export function _resetOtaSessions(): void {
  for (const t of timers.values()) clearTimeout(t);
  timers.clear();
  sessions.clear();
}
