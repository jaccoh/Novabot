/**
 * Fill an empty map database from the mower itself.
 *
 * The map lives on the mower (csv_file/ under maps/home0); the cloud and our
 * database only hold copies. A fresh install without a cloud import (or a
 * cloud import that brought no maps) starts with an empty database while the
 * mower still has its map. `get_map_outline {map_name:"all"}` makes mqtt_node
 * zip csv_file/ and POST it to uploadEquipmentMap, the request the official
 * app and our delete path already use.
 *
 * So when a mower connects and this server has no map for it, we ask once,
 * well after the connect: commands right after a connect crashed mqtt_node on
 * some mowers (broker.ts, "cloud-identiek"). No upload, or one without areas,
 * leaves state 'empty' and the dashboard asks for a backup; the next connect
 * tries again.
 */
import { EventEmitter } from 'events';

/** uploadEquipmentMap reports every processed mower ZIP here: (sn, areas). */
const uploads = new EventEmitter();
uploads.setMaxListeners(0);

export function notifyMapUpload(sn: string, areas: number): void {
  uploads.emit('upload', sn, areas);
}

/** Lets a module that may import the socket layer react to uploads, so the
 *  upload route itself does not (that import cycle breaks on load). */
export function onMapUpload(listener: (sn: string, areas: number) => void): void {
  uploads.on('upload', listener);
}

export type PullResult = { ok: true; areas: number } | { ok: false; reason: 'timeout' };

/** Listens before `send()` runs, so a fast upload is never missed. */
export function pullMapsFromMower(sn: string, send: () => void, timeoutMs: number): Promise<PullResult> {
  return new Promise(resolve => {
    const onUpload = (from: string, areas: number) => { if (from === sn) done({ ok: true, areas }); };
    const timer = setTimeout(() => done({ ok: false, reason: 'timeout' }), timeoutMs);
    function done(result: PullResult) {
      clearTimeout(timer);
      uploads.off('upload', onUpload);
      resolve(result);
    }
    uploads.on('upload', onUpload);
    send();
  });
}

export interface AutoPullDeps {
  online(sn: string): boolean;
  hasMaps(sn: string): boolean;
  /** A map operation, an unvalidated frame or a mapping session owns the map. */
  blocked(sn: string): boolean;
  send(sn: string): void;
  changed(sn: string): void;
}

export type AutoPullState = 'waiting' | 'pulling' | 'done' | 'empty';

export const AUTO_PULL_DELAY_MS = 90_000;
export const AUTO_PULL_TIMEOUT_MS = 120_000;

const states = new Map<string, AutoPullState>();

export function autoPullState(sn: string): AutoPullState | null {
  return states.get(sn) ?? null;
}

function setState(sn: string, state: AutoPullState | null, deps: AutoPullDeps): void {
  if (state) states.set(sn, state); else states.delete(sn);
  deps.changed(sn);
}

/** Called on every mower connect. ponytail: once per server run per mower
 *  unless the last try found nothing; a restart simply tries again. */
export function scheduleAutoMapPull(sn: string, deps: AutoPullDeps, delayMs = AUTO_PULL_DELAY_MS, timeoutMs = AUTO_PULL_TIMEOUT_MS): void {
  const now = states.get(sn);
  if (now === 'waiting' || now === 'pulling' || now === 'done') return;
  if (deps.hasMaps(sn)) return;
  setState(sn, 'waiting', deps);
  setTimeout(() => { void run(sn, deps, timeoutMs); }, delayMs);
}

async function run(sn: string, deps: AutoPullDeps, timeoutMs: number): Promise<void> {
  if (deps.hasMaps(sn)) { setState(sn, 'done', deps); return; }
  if (!deps.online(sn) || deps.blocked(sn)) { setState(sn, null, deps); return; }
  setState(sn, 'pulling', deps);
  const result = await pullMapsFromMower(sn, () => deps.send(sn), timeoutMs);
  setState(sn, result.ok && result.areas > 0 ? 'done' : 'empty', deps);
}
