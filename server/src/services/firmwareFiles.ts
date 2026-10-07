/**
 * Firmware files on disk: only bare names inside the firmware directory, and
 * only downloads from the manifest host.
 *
 * Express decodes %2F in a path parameter, so `/firmware/..%2F..%2Fdata%2Fx`
 * reached path.join as `../../data/x` and served any file on the box (the
 * JWT secret included). The download endpoints took `filename` and `url`
 * verbatim: a write anywhere on disk from any host.
 */
import path from 'path';

export const MANIFEST_HOST = 'downloads.ramonvanbruggen.nl';

/** `dir/<name>` when `name` is a bare file name, else null. */
export function safeFirmwarePath(dir: string, name: unknown): string | null {
  if (typeof name !== 'string' || !name || name !== path.basename(name) || name === '.' || name === '..') return null;
  const root = path.resolve(dir);
  const full = path.resolve(root, name);
  return full.startsWith(root + path.sep) ? full : null;
}

/** A firmware file name as the manifest lists them. */
export function isFirmwareFileName(name: unknown): name is string {
  return typeof name === 'string' && /^[A-Za-z0-9._-]+\.(deb|bin)$/.test(name) && !name.startsWith('.');
}

/** Firmware may only be pulled over https from the manifest host. */
export function firmwareSourceAllowed(url: unknown): boolean {
  if (typeof url !== 'string') return false;
  try {
    const u = new URL(url);
    return u.protocol === 'https:' && u.hostname === MANIFEST_HOST;
  } catch {
    return false;
  }
}
