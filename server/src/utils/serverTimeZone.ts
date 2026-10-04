const FALLBACK_TZ = 'Europe/Amsterdam';
let warned = false;

/**
 * The server's wall-clock zone: process.env.TZ when Intl accepts it, else
 * Europe/Amsterdam. A misspelt TZ (Europe/Bruxelles on a field install) made
 * Intl.DateTimeFormat throw on every saveCutGrassRecord, so no mow was saved.
 */
export function serverTimeZone(tz: string | undefined = process.env.TZ): string {
  if (!tz) return FALLBACK_TZ;
  try {
    new Intl.DateTimeFormat('en-CA', { timeZone: tz });
    return tz;
  } catch {
    if (!warned) {
      warned = true;
      console.warn(`[TZ] "${tz}" is geen geldige IANA-tijdzone (bijv. Europe/Brussels); ${FALLBACK_TZ} wordt gebruikt.`);
    }
    return FALLBACK_TZ;
  }
}
