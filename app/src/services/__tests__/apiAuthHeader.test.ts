import { describe, it, expect, vi, afterEach } from 'vitest';

import { ApiClient, setTokenProvider } from '../api';

/**
 * Dashboard calls (commands, maps, OTA) went out without any token, so past
 * the server's external auth gate every one of them got 401.
 */
describe('ApiClient Authorization header', () => {
  afterEach(() => { vi.restoreAllMocks(); });
  setTokenProvider(async () => 'stored-jwt');

  function captureFetch() {
    const calls: RequestInit[] = [];
    vi.stubGlobal('fetch', async (_url: string, init?: RequestInit) => {
      calls.push(init ?? {});
      return new Response(JSON.stringify([]), { status: 200, headers: { 'Content-Type': 'application/json' } });
    });
    return calls;
  }

  it('attaches the stored token to a plain dashboard call', async () => {
    const calls = captureFetch();
    await new ApiClient('https://example.invalid').getSchedules('LFIN1');
    expect((calls[0].headers as Record<string, string>)['Authorization']).toBe('stored-jwt');
  });

  it('keeps an explicitly passed token', async () => {
    const calls = captureFetch();
    await new ApiClient('https://example.invalid').getEquipmentList('explicit-jwt').catch(() => {});
    expect((calls[0].headers as Record<string, string>)['Authorization']).toBe('explicit-jwt');
  });
});
