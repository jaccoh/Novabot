import { afterEach, beforeEach, describe, expect, it, vi } from 'vitest';

/**
 * Pins the PROVEN provisioning behaviour of ble.ts, which is not to be
 * changed (Ramon, 2026-10-07): the whole mower sequence is sent in this
 * order with these timings, and a step the device does not answer does not
 * stop the sequence. The fake device speaks the real frame protocol
 * (ble_start / 20-byte chunks / ble_end, answers as notifications).
 */
const MOWER_SERVICE = '00000201-0000-1000-8000-00805f9b34fb';
const MOWER_NOTIFY = '00000021-0000-1000-8000-00805f9b34fb';
type Notify = (err: unknown, char: { uuid: string; value: string } | null) => void;

function fakeMower(opts: { silent?: string[] } = {}) {
  let notify: Notify | null = null;
  let buf = '';
  let collecting = false;
  const seen: string[] = [];
  const device = {
    id: 'mower',
    discoverAllServicesAndCharacteristics: vi.fn(async () => device),
    services: vi.fn(async () => [{
      uuid: MOWER_SERVICE,
      characteristics: async () => [{ uuid: MOWER_NOTIFY, isNotifiable: true, monitor: (cb: Notify) => { notify = cb; return { remove: vi.fn() }; } }],
    }]),
    monitorCharacteristicForService: vi.fn(),
    cancelConnection: vi.fn(async () => {}),
    readCharacteristicForService: vi.fn(async () => null),
    writeCharacteristicWithoutResponseForService: vi.fn(async (_svc: string, _char: string, b64: string) => {
      const s = Buffer.from(b64, 'base64').toString('utf8');
      if (s === 'ble_start') { collecting = true; buf = ''; return; }
      if (s !== 'ble_end') { if (collecting) buf += s; return; }
      collecting = false;
      const cmd = Object.keys(JSON.parse(buf))[0];
      seen.push(cmd);
      if (opts.silent?.includes(cmd)) return;
      const reply = JSON.stringify({ type: `${cmd}_respond`, message: { result: 0, value: null } });
      setTimeout(() => {
        for (const part of ['ble_start', reply, 'ble_end']) {
          notify?.(null, { uuid: MOWER_NOTIFY, value: Buffer.from(part, 'utf8').toString('base64') });
        }
      }, 50);
    }),
  };
  return { device, seen };
}

const manager = { connectToDevice: vi.fn(), onStateChange: vi.fn(), startDeviceScan: vi.fn(), stopDeviceScan: vi.fn() };
vi.mock('react-native-ble-plx', () => ({ BleManager: class { constructor() { return manager; } } }));
vi.mock('react-native', () => ({ Platform: { OS: 'ios' }, PermissionsAndroid: {} }));
// Imported dynamically for the device timezone; the real module drags in react-native.
vi.mock('expo-localization', () => ({ getCalendars: () => [{ timeZone: 'Europe/Amsterdam' }] }));

let ble: typeof import('../ble');
const params = { wifiSsid: 'home', wifiPassword: 'pw', mqttAddr: '192.168.0.247', mqttPort: 1883, lora: { addr: 718, channel: 16, hc: 20, lc: 14 } };

async function run(fake: ReturnType<typeof fakeMower>) {
  manager.connectToDevice.mockResolvedValue(fake.device);
  const progress: Array<[string, string]> = [];
  const pending = ble.provisionDevice('mower', 'mower', params, (phase, msg) => { progress.push([phase, msg]); });
  let result: Awaited<typeof pending> | null = null;
  pending.then(r => { result = r; });
  for (let i = 0; i < 120 && !result; i++) await vi.advanceTimersByTimeAsync(1000);
  return { result: result!, progress };
}

describe('provisionDevice (mower, fake device) — pins the proven behaviour', () => {
  beforeEach(async () => {
    vi.resetModules();
    vi.clearAllMocks();
    vi.useFakeTimers();
    vi.spyOn(console, 'log').mockImplementation(() => {});
    vi.spyOn(console, 'warn').mockImplementation(() => {});
    ble = await import('../ble');
  });
  afterEach(() => { vi.useRealTimers(); vi.restoreAllMocks(); });

  it('sends every step in the proven order and ends in done', async () => {
    const fake = fakeMower();
    const { result, progress } = await run(fake);
    expect(fake.seen).toEqual(['get_signal_info', 'set_wifi_info', 'set_lora_info', 'set_mqtt_info', 'set_cfg_info', 'set_robot_reboot']);
    expect(result.ok).toBe(true);
    expect(progress.at(-1)?.[0]).toBe('done');
    expect(fake.device.cancelConnection).toHaveBeenCalledTimes(1);
  });

  it('a step the device does not answer does not stop the sequence', async () => {
    const fake = fakeMower({ silent: ['set_mqtt_info'] });
    const { result } = await run(fake);
    expect(fake.seen).toEqual(['get_signal_info', 'set_wifi_info', 'set_lora_info', 'set_mqtt_info', 'set_cfg_info', 'set_robot_reboot']);
    expect(result.ok).toBe(true);
  });
});
