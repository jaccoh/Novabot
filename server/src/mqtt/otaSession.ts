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
  | 'downloading' | 'suspended' | 'unpacking' | 'installing'
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
  /** How much of the firmware file went out to the mower: the highest offset
   *  over its resumed requests. Stock 5.7.1 restarts its own percentage on
   *  every resume, so this is the download progress clients show; its absence
   *  means the mower has not fetched the file at all. */
  served?: { bytes: number; size: number };
}

const AWAITING_REBOOT_TIMEOUT_MS = 5 * 60_000;
/** Silence while downloading/unpacking/installing; every progress message restarts it. */
const PROGRESS_SILENCE_MS = 10 * 60_000;
const FINISHED_TTL_MS = 10 * 60_000;
const TERMINAL: ReadonlySet<OtaPhase> = new Set(['done', 'rolled-back', 'failed', 'stalled']);
const IN_PROGRESS: ReadonlySet<OtaPhase> = new Set(['downloading', 'unpacking', 'installing']);

const sessions = new Map<string, OtaSession>();
const timers = new Map<string, NodeJS.Timeout>();
/** Sessions the user stopped following; their state reports stay quiet until a new update starts. */
const cancelled = new Set<string>();

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
  cancelled.delete(sn);
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
  // ota_client_node says 'fail'; 'failed' and 'error' are kept for other senders.
  if (status === 'fail' || status === 'failed' || status === 'error') return setPhase(sn, 'failed');
  // The download pauses while the mower is off its charger.
  if (status === 'suspend') return setPhase(sn, 'suspended');
  // While the file is still going out, this is the download, whatever the
  // mower's own counter says.
  if (s.served && s.served.bytes < s.served.size) return setPhase(sn, 'downloading');
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

/** The firmware route hands out part of `filename`: progress for the session
 *  that waits for that version, and proof the mower reaches this server. */
export function otaSessionServed(filename: string, bytes: number, size: number): void {
  for (const s of sessions.values()) {
    if (TERMINAL.has(s.phase) || !fileHasVersion(filename, s.target)) continue;
    s.served = { bytes: Math.max(s.served?.bytes ?? 0, bytes), size };
    if (s.phase === 'suspended') { setPhase(s.sn, 'downloading'); continue; }
    armTimer(s.sn, s.phase);
    emitOtaEvent(s.sn, 'phase', { ...s });
  }
}

/** 'mower_firmware_v6.0.2-custom-46.deb' holds v6.0.2-custom-46, not v6.0.2-custom-4. */
function fileHasVersion(filename: string, version: string): boolean {
  const v = norm(version).replace(/[.*+?^${}()|[\]\\]/g, '\\$&');
  return v !== '' && new RegExp(`(^|[^0-9.])v?${v}(?![0-9])`).test(filename);
}

/** Stop following an update. A mower cannot drop an accepted update (no such
 *  command); it holds it until it restarts. This only ends the session here. */
export function otaSessionCancel(sn: string): boolean {
  const had = sessions.delete(sn);
  clearTimeout(timers.get(sn));
  timers.delete(sn);
  cancelled.add(sn);
  emitOtaEvent(sn, 'cancelled', { sn });
  return had;
}

export function otaSessionCancelled(sn: string): boolean {
  return cancelled.has(sn);
}

export function getOtaSession(sn: string): OtaSession | undefined {
  return sessions.get(sn);
}

export function _resetOtaSessions(): void {
  for (const t of timers.values()) clearTimeout(t);
  timers.clear();
  sessions.clear();
  cancelled.clear();
}
