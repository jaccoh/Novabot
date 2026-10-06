import { describe, it, expect, vi, afterEach } from 'vitest';
import { notifyMapUpload, pullMapsFromMower, scheduleAutoMapPull, autoPullState, type AutoPullDeps } from '../../services/mapPull.js';

afterEach(() => { vi.useRealTimers(); });

function deps(over: Partial<AutoPullDeps> = {}): AutoPullDeps {
  return { online: () => true, hasMaps: () => false, blocked: () => false, send: () => {}, changed: () => {}, ...over };
}

describe('pullMapsFromMower', () => {
  it('resolves with the area count of the matching upload, even when it arrives during send()', async () => {
    await expect(pullMapsFromMower('LFIN1', () => notifyMapUpload('LFIN1', 4), 1000)).resolves.toEqual({ ok: true, areas: 4 });
  });

  it('ignores another mower and times out', async () => {
    vi.useFakeTimers();
    const pending = pullMapsFromMower('LFIN1', () => notifyMapUpload('LFIN2', 3), 1000);
    await vi.advanceTimersByTimeAsync(1000);
    await expect(pending).resolves.toEqual({ ok: false, reason: 'timeout' });
  });
});

describe('scheduleAutoMapPull', () => {
  it('pulls once after the delay when the database has no map', async () => {
    vi.useFakeTimers();
    const send = vi.fn((sn: string) => notifyMapUpload(sn, 2));
    scheduleAutoMapPull('LFIN10', deps({ send }), 100, 1000);
    expect(autoPullState('LFIN10')).toBe('waiting');
    scheduleAutoMapPull('LFIN10', deps({ send }), 100, 1000); // a reconnect while waiting
    await vi.advanceTimersByTimeAsync(100);
    expect(send).toHaveBeenCalledOnce();
    expect(autoPullState('LFIN10')).toBe('done');
  });

  it('leaves the mower alone when the database already has its map', () => {
    const send = vi.fn();
    scheduleAutoMapPull('LFIN11', deps({ hasMaps: () => true, send }), 0, 1000);
    expect(autoPullState('LFIN11')).toBeNull();
    expect(send).not.toHaveBeenCalled();
  });

  it('marks empty when nothing comes back, and tries again on the next connect', async () => {
    vi.useFakeTimers();
    const send = vi.fn();
    scheduleAutoMapPull('LFIN12', deps({ send }), 100, 1000);
    await vi.advanceTimersByTimeAsync(1100);
    expect(autoPullState('LFIN12')).toBe('empty');
    scheduleAutoMapPull('LFIN12', deps({ send }), 100, 1000);
    expect(autoPullState('LFIN12')).toBe('waiting');
  });

  it('does not send while a mapping session or map operation owns the map', async () => {
    vi.useFakeTimers();
    const send = vi.fn();
    scheduleAutoMapPull('LFIN13', deps({ blocked: () => true, send }), 100, 1000);
    await vi.advanceTimersByTimeAsync(100);
    expect(send).not.toHaveBeenCalled();
    expect(autoPullState('LFIN13')).toBeNull();
  });
});
