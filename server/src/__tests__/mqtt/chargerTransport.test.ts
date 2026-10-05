/**
 * A charger OTA (v0.3.0 → v0.4.0) left equipment.charger_version on v0.3.0:
 * the server asks no version on connect. Every command then went out plain
 * and the AES-only v0.4.0 charger ignored it (field report 2026-10-05). The
 * server now follows what the charger itself sends, and asks its version
 * once when that disagrees with equipment.
 */
import { describe, it, expect, vi, beforeEach, afterEach } from 'vitest';

vi.mock('../../mqtt/broker.js', () => ({
  isSnBanned: () => false,
  isDeviceOnline: () => true,
}));

import { initMapSync, publishToDevice, noteChargerTransport, _resetChargerTransport } from '../../mqtt/mapSync.js';
import { tryDecrypt } from '../../mqtt/decrypt.js';
import { equipmentRepo } from '../../db/repositories/index.js';

const SN = 'LFIC1231000278';
let sent: Buffer[] = [];
// What reached the topic, and whether it went out encrypted.
const sentAs = () => sent.map(b => {
  const plain = tryDecrypt(b, SN);
  return plain ? `aes ${plain}` : `plain ${b.toString('utf8')}`;
});

beforeEach(() => {
  vi.useFakeTimers();
  _resetChargerTransport();
  sent = [];
  initMapSync({ publish: (p: { payload: Buffer }, cb: () => void) => { sent.push(p.payload); cb(); } } as never);
  equipmentRepo.create({ equipment_id: 'eq-ota', mower_sn: 'LFIN1231000298', charger_sn: SN, charger_version: 'v0.3.0' });
});
afterEach(() => { vi.useRealTimers(); });

describe('charger transport after a firmware change', () => {
  it('asks the version once, encrypted, when a "v0.3.0" charger talks AES', () => {
    noteChargerTransport(SN, true);
    noteChargerTransport(SN, true);
    expect(sentAs()).toEqual(['aes {"ota_version_info":null}']);

    // Every other command follows the charger too, not the stale version.
    publishToDevice(SN, { get_lora_info: null });
    expect(sentAs()[1]).toBe('aes {"get_lora_info":null}');

    // No answer: ask again, but not before the gap.
    vi.advanceTimersByTime(10 * 60_000);
    noteChargerTransport(SN, true);
    expect(sent).toHaveLength(3);
  });

  it('stays quiet when the charger matches equipment', () => {
    noteChargerTransport(SN, false);
    expect(sent).toEqual([]);
    equipmentRepo.updateChargerVersionByChargerSn(SN, 'v0.4.0');
    noteChargerTransport(SN, true);
    expect(sent).toEqual([]);
  });
});
