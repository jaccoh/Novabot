import { describe, it, expect } from 'vitest';
import { insertEvent } from '../../../../dashboard/src/utils/mowerEvents.js';
import type { MowerEvent } from '../../../../dashboard/src/types/index.js';

const ev = (ts: number, type: MowerEvent['type'] = 'docked'): MowerEvent =>
  ({ sn: 'SN', type, ts, title: '', message: '', data: {} });

// #141: GET /api/events/:sn returns newest first and the bell fed it into the
// list one by one, each prepended, so the oldest ended on top.
describe('insertEvent', () => {
  it('keeps newest first whatever order the backlog arrives in', () => {
    let list: MowerEvent[] = [];
    for (const e of [ev(300), ev(200), ev(100)]) list = insertEvent(list, e, 100);
    expect(list.map(e => e.ts)).toEqual([300, 200, 100]);
    list = insertEvent(list, ev(400), 100);
    expect(list.map(e => e.ts)).toEqual([400, 300, 200, 100]);
  });

  it('drops duplicates and caps the list', () => {
    let list = insertEvent([], ev(100), 2);
    list = insertEvent(list, ev(100), 2);
    expect(list).toHaveLength(1);
    list = insertEvent(list, ev(300), 2);
    list = insertEvent(list, ev(200), 2);
    expect(list.map(e => e.ts)).toEqual([300, 200]);
  });
});
