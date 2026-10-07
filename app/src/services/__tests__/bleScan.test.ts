import { afterEach, beforeEach, describe, expect, it, vi } from 'vitest';

const manager = {
  state: 'PoweredOn' as string,
  onStateChange: vi.fn((cb: (s: string) => void, _emit: boolean) => { cb(manager.state); return { remove: vi.fn() }; }),
  startDeviceScan: vi.fn(),
  stopDeviceScan: vi.fn(),
};
vi.mock('react-native-ble-plx', () => ({ BleManager: class { constructor() { return manager; } } }));
vi.mock('react-native', () => ({ Platform: { OS: 'ios' }, PermissionsAndroid: {} }));

let ble: typeof import('../ble');

/** A scan must always end: with Bluetooth off no timer ever started, so the
 *  spinner ran forever and the mapping screen stayed busy. */
describe('scanForDevices ends', () => {
  beforeEach(async () => {
    vi.resetModules();
    vi.clearAllMocks();
    vi.useFakeTimers();
    vi.spyOn(console, 'log').mockImplementation(() => {});
    ble = await import('../ble');
  });
  afterEach(() => { vi.useRealTimers(); vi.restoreAllMocks(); });

  it('calls onDone when Bluetooth is off', async () => {
    manager.state = 'PoweredOff';
    const onDone = vi.fn();
    ble.scanForDevices(5000, vi.fn(), onDone);
    await vi.advanceTimersByTimeAsync(10);
    expect(onDone).toHaveBeenCalledTimes(1);
    expect(manager.startDeviceScan).not.toHaveBeenCalled();
  });

  it('scans and calls onDone after the duration when Bluetooth is on', async () => {
    manager.state = 'PoweredOn';
    const onDone = vi.fn();
    ble.scanForDevices(5000, vi.fn(), onDone);
    await vi.advanceTimersByTimeAsync(10);
    expect(manager.startDeviceScan).toHaveBeenCalledTimes(1);
    expect(onDone).not.toHaveBeenCalled();
    await vi.advanceTimersByTimeAsync(5000);
    expect(onDone).toHaveBeenCalledTimes(1);
  });
});
