import type { MowerEvent } from '../types';

/** Newest first, whatever order the socket and the backlog deliver in: the
 *  backlog (GET /api/events/:sn) comes newest first and was fed in one by one,
 *  each prepended, so the oldest ended on top (#141). */
export function insertEvent(prev: MowerEvent[], e: MowerEvent, max: number): MowerEvent[] {
  if (prev.some(p => p.sn === e.sn && p.type === e.type && p.ts === e.ts)) return prev;
  return [e, ...prev].sort((a, b) => b.ts - a.ts).slice(0, max);
}
