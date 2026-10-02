import { beforeEach, expect, it, vi } from 'vitest';
import { readFileSync, rmSync } from 'node:fs';
import path from 'node:path';
import unzipper from 'unzipper';

vi.mock('../../mqtt/broker.js', () => ({ isDeviceOnline: () => true }));
vi.mock('../../services/mowerFileCapability.js', () => ({ isOpenNovaMower: () => true }));
vi.mock('../../services/mapBackup.js', () => ({ scheduleSnapshot: vi.fn() }));
vi.mock('../../mqtt/sensorData.js', () => ({ deviceCache: new Map(), getDockPose: vi.fn() }));
vi.mock('../../dashboard/socketHandler.js', () => ({ forwardToDashboard: vi.fn() }));
vi.mock('../../services/mowerMapApply.js', async original => ({
  ...await original<object>(), installVerifiedMapZip: vi.fn(),
}));

import { installZoneCopy } from '../../services/installZoneCopy.js';
import { installVerifiedMapZip } from '../../services/mowerMapApply.js';
import { isMowerMapOperationBusy, withMowerMapOperation } from '../../services/mowerMapOperation.js';
import { clearPositionTelemetry, ingestPositionTelemetry } from '../../services/positionTelemetry.js';
import { clearFrameUnvalidated, clearMapInstallPending, isFrameUnvalidated, isMapInstallPending, loadFrameValidationFromDb, markMapInstallPending } from '../../services/frameValidation.js';
import { mapRepo, deviceSettingsRepo } from '../../db/repositories/index.js';
import { getPhotoDockPose, PHOTO_DOCK_KEY } from '../../services/dockPhotoReference.js';
import { planZoneCopy } from '../../services/zoneCopy.js';

const sn = 'LFIN_INSTALL_COPY', source = 'LFIN_INSTALL_SOURCE';
const dock = { x: .03, y: .73, orientation: -Math.PI / 2 };
const work = [{ x: -3, y: 0 }, { x: 3, y: 0 }, { x: 3, y: 8 }, { x: -3, y: 8 }];
const csv: Record<string, string> = {
  'map0_work.csv': '-3.000000,0\n3,0\n3,8\n-3,8\n',
  'map0tocharge_unicom.csv': '.03,.73\n.03,1.93\n',
  'map_info.json': JSON.stringify({ charging_pose: dock, 'map0_work.csv': { map_size: 48 } }),
};
const snapshot = () => ({ result: 0, snapshot_consistent: true, csv_files: { ...csv }, x3_csv_files: { ...csv },
  pos_json: '{"utm_origin":{"x":1,"y":2,"z":0,"utm_zone":32}}',
  charging_station_yaml: `charging_pose: [${dock.x},${dock.y},${dock.orientation}]` });
const plan = () => planZoneCopy({ slot: 1, work: work.map(p => ({ x: p.x + 8, y: p.y })), obstacles: [], existing: [], dock, dockChannelRowExists: false });
let installedSnapshot: Record<string, unknown>;
const verifyRuntime = vi.fn(async () => installedSnapshot);
const verifyBefore = vi.fn(async () => ({ verifyRuntime }));
const run = (copyPlan = plan(), targetSnapshot = snapshot()) => withMowerMapOperation(sn, targetOperation => withMowerMapOperation(source, sourceOperation =>
  installZoneCopy(sn, copyPlan, { alias: 'Kopie', acceptChannel: true }, {
    source: dock, target: dock, sourceSnapshot: snapshot(), targetSnapshot, sourceOperation, targetOperation,
  }, verifyBefore)));

beforeEach(() => {
  vi.clearAllMocks();
  verifyBefore.mockImplementation(async () => ({ verifyRuntime }));
  verifyRuntime.mockImplementation(async () => installedSnapshot);
  rmSync(path.join(process.env.STORAGE_PATH!, 'zone-copy'), { recursive: true, force: true });
  for (const target of [sn, source]) { clearFrameUnvalidated(target); clearMapInstallPending(target); clearPositionTelemetry(target); }
  for (let i = 0; i < 8; i++) ingestPositionTelemetry(sn, {
    rtk_fix_quality: 4, localization_state: 'RUNNING', recharge_status: 9, map_position_x: dock.x, map_position_y: dock.y,
  }, Date.now() - 8000 + i * 1000);
  mapRepo.create({ map_id: `${sn}-original`, mower_sn: sn, map_type: 'work', canonical_name: 'map0', map_area: JSON.stringify(work) });
  mapRepo.create({ map_id: `${sn}-channel`, mower_sn: sn, map_type: 'unicom', canonical_name: 'map0tocharge_unicom',
    map_area: JSON.stringify([{ x: dock.x, y: dock.y }, { x: dock.x, y: 1.93 }]) });
  vi.mocked(installVerifiedMapZip).mockImplementation(async (_sn, input) => {
    expect(isMowerMapOperationBusy(sn)).toBe(true);
    expect(isMowerMapOperationBusy(source)).toBe(true);
    expect(mapRepo.findBySnAndCanonical(sn, 'map1')).toBeUndefined();
    markMapInstallPending(sn);
    const pgm = Buffer.concat([Buffer.from('P5\n3 3\n255\n'), Buffer.alloc(9, 254)]).toString('base64');
    const slots = [...input.expectedCsv.keys()].filter(n => /^map\d+_work\.csv$/.test(n)).map(n => n.slice(0, -9));
    installedSnapshot = { ...input.before, csv_files: Object.fromEntries(input.expectedCsv), x3_csv_files: Object.fromEntries(input.expectedCsv),
      map_files_b64: Object.fromEntries(['map', ...slots].map(n => [`${n}.pgm`, pgm])),
      map_files_text: Object.fromEntries(['map', ...slots].map(n => [`${n}.yaml`, `image: ${n}.pgm`])) };
    return installedSnapshot;
  });
});

it('keeps both leases until post-install verification, then commits while preserving old CSVs and the photo reference', async () => {
  deviceSettingsRepo.upsert(sn, PHOTO_DOCK_KEY, JSON.stringify(dock));
  verifyRuntime.mockImplementation(async () => {
    expect(isMowerMapOperationBusy(sn)).toBe(true); expect(isMowerMapOperationBusy(source)).toBe(true);
    expect(isFrameUnvalidated(sn)).toBe(true);
    expect(mapRepo.findBySnAndCanonical(sn, 'map1')).toBeUndefined();
    return installedSnapshot;
  });
  const saved = await run();
  expect(saved.mapId).toContain(sn);
  expect(mapRepo.findBySnAndCanonical(sn, 'map1')?.map_name).toBe('Kopie');
  expect(isFrameUnvalidated(sn)).toBe(false);
  expect(isMowerMapOperationBusy(sn)).toBe(false); expect(isMowerMapOperationBusy(source)).toBe(false);
  expect(getPhotoDockPose(sn)).toEqual(dock);
  expect(verifyBefore).toHaveBeenCalledOnce(); expect(verifyRuntime).toHaveBeenCalledOnce();
  const input = vi.mocked(installVerifiedMapZip).mock.calls[0][1];
  const zip = await unzipper.Open.buffer(input.bytes);
  expect((await zip.files.find(f => f.path === 'csv_file/map0_work.csv')!.buffer()).toString()).toBe(csv['map0_work.csv']);
  expect(JSON.parse(input.expectedCsv.get('map_info.json')!).charging_pose).toEqual(dock);
  expect(readFileSync(path.join(process.env.STORAGE_PATH!, 'maps', `${sn}_latest.zip`))).toEqual(input.bytes);
});

it('replaces one native slot without renumbering or changing another slot', async () => {
  const old = snapshot();
  old.csv_files = old.x3_csv_files = {
    ...old.csv_files,
    'map0_0_obstacle.csv': '-2,2\n-1,2\n-1,3\n',
    'map0_1_obstacle.csv': '1,2\n2,2\n2,3\n',
    'map0tomap3_0_unicom.csv': '0,4\n8,4\n',
    'map3_work.csv': '8,0\n12,0\n12,4\n8,4\n',
  };
  old.csv_files['map_info.json'] = old.x3_csv_files['map_info.json'] = JSON.stringify({
    charging_pose: dock, 'map0_work.csv': { map_size: 48 }, 'map3_work.csv': { map_size: 16 },
  });
  mapRepo.create({ map_id: `${sn}-old-obstacle-0`, mower_sn: sn, map_type: 'obstacle', canonical_name: 'map0_0_obstacle', map_area: '[]' });
  mapRepo.create({ map_id: `${sn}-old-obstacle-1`, mower_sn: sn, map_type: 'obstacle', canonical_name: 'map0_1_obstacle', map_area: '[]' });
  mapRepo.create({ map_id: `${sn}-old-link`, mower_sn: sn, map_type: 'unicom', canonical_name: 'map0tomap3_0_unicom', map_area: '[]' });
  mapRepo.create({ map_id: `${sn}-map3`, mower_sn: sn, map_type: 'work', canonical_name: 'map3', map_area: JSON.stringify(work) });
  const replacement = planZoneCopy({ slot: 0, replacesExisting: true, work, obstacles: [], existing: [], dock, dockChannelRowExists: true });
  await run(replacement, old);
  const sent = vi.mocked(installVerifiedMapZip).mock.calls[0][1].expectedCsv;
  expect([...sent.keys()].sort()).toEqual(['map0_work.csv', 'map0tocharge_unicom.csv', 'map3_work.csv', 'map_info.json']);
  expect(sent.get('map3_work.csv')).toBe(old.csv_files['map3_work.csv']);
  expect(mapRepo.findBySnAndCanonical(sn, 'map3')?.map_id).toBe(`${sn}-map3`);
  expect(mapRepo.findByMowerSn(sn).map(r => r.canonical_name).sort()).toEqual(['map0', 'map0tocharge_unicom', 'map3']);
});

it('accepts a connector filtered in csv_file while x3_csv_file keeps the route, and ships the full route', async () => {
  const s = snapshot();
  s.csv_files = { ...s.csv_files, 'map3_work.csv': '8,0\n12,0\n12,4\n8,4\n', 'map0tomap3_0_unicom.csv': '' };
  s.x3_csv_files = { ...s.csv_files, 'map0tomap3_0_unicom.csv': '3,4\n8,4\n' };
  mapRepo.create({ map_id: `${sn}-map3`, mower_sn: sn, map_type: 'work', canonical_name: 'map3', map_area: JSON.stringify(work) });
  mapRepo.create({ map_id: `${sn}-link`, mower_sn: sn, map_type: 'unicom', canonical_name: 'map0tomap3_0_unicom', map_area: '[]' });
  await run(plan(), s);
  expect(vi.mocked(installVerifiedMapZip).mock.calls[0][1].expectedCsv.get('map0tomap3_0_unicom.csv')).toBe('3,4\n8,4\n');
  const conflict = snapshot();
  conflict.csv_files = { ...conflict.csv_files, 'map0tomap3_0_unicom.csv': '9,9\n' };
  conflict.x3_csv_files = { ...conflict.csv_files, 'map0tomap3_0_unicom.csv': '3,4\n8,4\n' };
  await expect(run(plan(), conflict)).rejects.toThrow('kaartkopieën');
});

it('rejects changed runtime before transfer without any device or DB write', async () => {
  const before = mapRepo.findByMowerSn(sn);
  verifyBefore.mockRejectedValueOnce(new Error('Runtime offset changed by 7.44 cm'));
  await expect(run()).rejects.toThrow('7.44 cm');
  expect(installVerifiedMapZip).not.toHaveBeenCalled();
  expect(mapRepo.findByMowerSn(sn)).toEqual(before);
  expect(isFrameUnvalidated(sn)).toBe(false);
});

it.each(['runtime', 'unknown', 'rasters', 'late_files', 'late_rasters', 'late_origin'])('keeps failed %s installs out of the DB and keeps navigation blocked even after restart', async kind => {
  const before = mapRepo.findByMowerSn(sn);
  deviceSettingsRepo.upsert(sn, PHOTO_DOCK_KEY, JSON.stringify(dock));
  if (kind === 'runtime') verifyRuntime.mockRejectedValueOnce(new Error('Runtime offset changed'));
  if (kind === 'late_files') verifyRuntime.mockImplementationOnce(async () => ({ ...installedSnapshot, csv_files: csv }));
  if (kind === 'late_rasters') verifyRuntime.mockImplementationOnce(async () => ({ ...installedSnapshot, map_files_b64: {} }));
  if (kind === 'late_origin') verifyRuntime.mockImplementationOnce(async () => ({ ...installedSnapshot, pos_json: '{"origin":"changed"}' }));
  if (kind === 'unknown') vi.mocked(installVerifiedMapZip).mockImplementationOnce(async () => { markMapInstallPending(sn); return null; });
  if (kind === 'rasters') vi.mocked(installVerifiedMapZip).mockImplementationOnce(async () => { markMapInstallPending(sn); return { map_files_b64: {} }; });
  await expect(run()).rejects.toThrow();
  expect(mapRepo.findByMowerSn(sn)).toEqual(before);
  expect(getPhotoDockPose(sn)).toEqual(dock);
  expect(isFrameUnvalidated(sn)).toBe(true);
  loadFrameValidationFromDb();
  expect(isFrameUnvalidated(sn)).toBe(true);
  // Even a verified reinstall cannot release failed runtime/origin validation.
  clearMapInstallPending(sn);
  expect(isFrameUnvalidated(sn)).toBe(kind === 'runtime' || kind === 'late_origin');
});

it('accepts a target whose primary map_info.json the firmware rewrote in its own format', async () => {
  const target = snapshot();
  // novabot_mapping's formatting of the same dock pose, map_size recomputed.
  target.csv_files['map_info.json'] = `{\n   "charging_pose" : {\n      "orientation" : ${dock.orientation},\n      "x" : 0.029999999999999999,\n      "y" : 0.72999999999999998\n   },\n`
    + '   "map0_work.csv" : {\n      "map_size" : 48.100000000000001\n   }\n}\n';
  await run(plan(), target);
  expect(installVerifiedMapZip).toHaveBeenCalledTimes(1);
  const sent = JSON.parse(vi.mocked(installVerifiedMapZip).mock.calls[0][1].expectedCsv.get('map_info.json')!);
  expect(sent.charging_pose).toEqual(dock);
  expect(Object.keys(sent).sort()).toEqual(['charging_pose', 'map0_work.csv', 'map1_work.csv']);
});

it('retries after a failed install and replaces the slot files that install left behind', async () => {
  markMapInstallPending(sn);
  const target = snapshot();
  const leftover = { 'map1_work.csv': '4,0\n10,0\n10,8\n4,8\n', 'map1tocharge_unicom.csv': '.03,.73\n4,.73\n',
    'map2_work.csv': '20,0\n24,0\n24,4\n', 'map2_0_obstacle.csv': '21,1\n22,1\n22,2\n' };
  const info = JSON.stringify({ charging_pose: dock, 'map0_work.csv': { map_size: 48 }, 'map1_work.csv': { map_size: 48 }, 'map2_work.csv': { map_size: 8 } });
  target.csv_files = { ...target.csv_files, ...leftover, 'map_info.json': info };
  target.x3_csv_files = { ...target.x3_csv_files, ...leftover, 'map_info.json': info };
  await run(plan(), target);
  const sent = vi.mocked(installVerifiedMapZip).mock.calls[0][1].expectedCsv;
  expect(sent.get('map1_work.csv')).toBe('5.000000,0.000000\n11.000000,0.000000\n11.000000,8.000000\n5.000000,8.000000\n');
  expect([...sent.keys()].filter(n => n.startsWith('map2'))).toEqual([]);
  expect(Object.keys(JSON.parse(sent.get('map_info.json')!)).sort()).toEqual(['charging_pose', 'map0_work.csv', 'map1_work.csv']);
  expect(mapRepo.findBySnAndCanonical(sn, 'map1')).toBeDefined();
  expect(isMapInstallPending(sn)).toBe(false);
});

it('accepts the firmware rewriting map_info.json between the install and the final check', async () => {
  verifyRuntime.mockImplementationOnce(async () => {
    const installed = installedSnapshot.csv_files as Record<string, string>;
    const zones = Object.keys(JSON.parse(installed['map_info.json'])).filter(k => k !== 'charging_pose').reverse();
    const rewritten = `{\n   "charging_pose" : {\n      "orientation" : ${dock.orientation},\n      "x" : 0.029999999999999999,\n      "y" : 0.72999999999999998\n   },\n`
      + zones.map(k => `   "${k}" : {\n      "map_size" : 1\n   }`).join(',\n') + '\n}\n';
    return { ...installedSnapshot, csv_files: { ...installed, 'map_info.json': rewritten } };
  });
  await run();
  expect(mapRepo.findBySnAndCanonical(sn, 'map1')).toBeDefined();
});
