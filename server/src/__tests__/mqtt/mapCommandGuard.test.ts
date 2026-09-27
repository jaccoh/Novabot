import { createCipheriv } from 'node:crypto';
import { afterEach, describe, expect, it, vi } from 'vitest';
vi.mock('../../mqtt/broker.js', () => ({ isSnBanned: () => false, isDeviceOnline: () => true }));
vi.mock('../../dashboard/socketHandler.js', () => ({ emitDeviceBound: vi.fn(), emitDevicePaired: vi.fn() }));
vi.mock('../../mqtt/sensorData.js', () => ({ deviceCache: new Map() }));
import { isMapMqttPacketBlocked } from '../../mqtt/mapCommandGuard.js';
import { withMowerMapOperation } from '../../services/mowerMapOperation.js';
import { markFrameUnvalidated, clearFrameUnvalidated } from '../../services/frameValidation.js';
import { handleMapMessage, handleExtendedResponse, onExtendedResponse, offExtendedResponse } from '../../mqtt/mapSync.js';
import { mapRepo } from '../../db/repositories/index.js';

const SN = 'LFIN_GUARD_0238';
const topic = `Dart/Send_mqtt/${SN}`;
const start = JSON.stringify({ start_navigation: { cmd_num: 1 } });
function encrypt(json: string): Buffer {
  const bytes = Buffer.from(json);
  const padded = Buffer.alloc(Math.ceil(bytes.length / 16) * 16);
  bytes.copy(padded);
  const cipher = createCipheriv('aes-128-cbc', Buffer.from(`abcdabcd1234${SN.slice(-4)}`), Buffer.from('abcd1234abcd1234'));
  cipher.setAutoPadding(false);
  return Buffer.concat([cipher.update(padded), cipher.final()]);
}

describe('direct app MQTT map guard', () => {
  afterEach(() => { clearFrameUnvalidated(SN); vi.clearAllTimers(); vi.useRealTimers(); });

  it('blocks plaintext, encrypted and extended autonomous commands during a lease, while allowing manual control and stops', async () => {
    await withMowerMapOperation(SN, async () => {
      expect(isMapMqttPacketBlocked(topic, start)).toBe(true);
      expect(isMapMqttPacketBlocked(topic, encrypt(start))).toBe(true);
      expect(isMapMqttPacketBlocked(`novabot/extended/${SN}`, JSON.stringify({ mow_zone: {} }))).toBe(true);
      expect(isMapMqttPacketBlocked(`novabot/extended/${SN}`, JSON.stringify({ write_map_files: {} }))).toBe(true);
      expect(isMapMqttPacketBlocked(topic, JSON.stringify({ set_remote_control: { linear: 0 }, stop_navigation: {} }))).toBe(false);
      expect(isMapMqttPacketBlocked(`Dart/Receive_mqtt/${SN}`, start)).toBe(false);
    });
    expect(isMapMqttPacketBlocked(topic, start)).toBe(false);
  });

  it('still blocks app navigation after an interrupted operation has invalidated the frame', () => {
    markFrameUnvalidated(SN);
    expect(isMapMqttPacketBlocked(topic, encrypt(start))).toBe(true);
    expect(isMapMqttPacketBlocked(topic, JSON.stringify({ stop_navigation: {} }))).toBe(false);
  });

  it.each(['busy', 'unvalidated'])('ignores passive map mutations while %s but still delivers operation responses', async state => {
    vi.useFakeTimers();
    const check = async () => {
      expect(handleMapMessage(SN, { get_map_list_respond: { maps: [{ map_id: 'incoming-map', map_name: 'Incoming' }] } })).toBe(true);
      expect(handleMapMessage(SN, { report_state_map_outline: {
        map_id: 'incoming-outline', map_position: [{ lat: 52, lng: 5 }, { lat: 52.01, lng: 5.01 }, { lat: 52, lng: 5.01 }],
      } })).toBe(true);
      expect(mapRepo.findByMowerSn(SN)).toEqual([]);
      const handler = vi.fn();
      onExtendedResponse(SN, handler);
      try {
        const response = { sync_map_respond: { result: 0, operation_id: 'owned-operation' } };
        handleExtendedResponse(SN, JSON.stringify(response));
        expect(handler).toHaveBeenCalledExactlyOnceWith(response);
      } finally { offExtendedResponse(SN, handler); }
    };
    if (state === 'busy') await withMowerMapOperation(SN, check);
    else { markFrameUnvalidated(SN); await check(); }
    clearFrameUnvalidated(SN);
    handleMapMessage(SN, { get_map_list_respond: { maps: [{ map_id: 'confirmed-map', map_name: 'Confirmed' }] } });
    expect(mapRepo.findByMowerSn(SN).map(row => row.map_id)).toEqual(['confirmed-map']);
  });
});
