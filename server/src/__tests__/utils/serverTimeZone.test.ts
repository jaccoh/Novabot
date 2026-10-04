import { expect, it } from 'vitest';
import { serverTimeZone } from '../../utils/serverTimeZone.js';

it('keeps a valid IANA zone and falls back when TZ is unset or misspelt', () => {
  expect(serverTimeZone('Europe/Brussels')).toBe('Europe/Brussels');
  expect(serverTimeZone(undefined)).toBe('Europe/Amsterdam');
  // A field install had TZ=Europe/Bruxelles: Intl threw on every mow record.
  const tz = serverTimeZone('Europe/Bruxelles');
  expect(tz).toBe('Europe/Amsterdam');
  expect(() => new Intl.DateTimeFormat('en-CA', { timeZone: tz }).format(new Date())).not.toThrow();
});
