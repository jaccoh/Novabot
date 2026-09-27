import { describe, it, expect, vi } from 'vitest';
vi.mock('../../mqtt/broker.js', () => ({ isDeviceOnline: () => true }));
import { assertReanchorFiles } from '../../services/reanchorGps.js';

describe('reanchor file preservation', () => {
  const before = { result: 0, snapshot_consistent: true, pos_json: 'old', charging_station_yaml: 'dock',
    csv_files: { 'map0_work.csv': '0,0' }, x3_csv_files: {}, map_files_b64: {}, map_files_text: {} };
  it('allows only the origin to change', () => {
    expect(() => assertReanchorFiles(before, { ...before, pos_json: 'new' })).not.toThrow();
    for (const key of ['charging_station_yaml', 'csv_files', 'x3_csv_files', 'map_files_b64', 'map_files_text']) {
      expect(() => assertReanchorFiles(before, { ...before, [key]: undefined })).toThrow();
      expect(() => assertReanchorFiles(before, { ...before, [key]: 'changed' })).toThrow();
    }
    expect(() => assertReanchorFiles(before, { ...before, snapshot_consistent: false })).toThrow();
    expect(() => assertReanchorFiles(before, null)).toThrow();
  });
});
