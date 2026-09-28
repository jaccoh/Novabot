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
import { applyMapsToMower, installVerifiedMapZip } from '../../services/mowerMapApply.js';
import { isMowerMapOperationBusy, withMowerMapOperation } from '../../services/mowerMapOperation.js';
import { clearPositionTelemetry, ingestPositionTelemetry } from '../../services/positionTelemetry.js';
import { clearFrameUnvalidated, isFrameUnvalidated, loadFrameValidationFromDb, markFrameUnvalidated } from '../../services/frameValidation.js';
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
  for (const target of [sn, source]) { clearFrameUnvalidated(target); clearPositionTelemetry(target); }
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
    markFrameUnvalidated(sn, { preservePhotoDock: true });
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

it('rejects changed runtime before transfer without any device or DB write', async () => {
  const before = mapRepo.findByMowerSn(sn);
  verifyBefore.mockRejectedValueOnce(new Error('Runtime offset changed by 7.44 cm'));
  await expect(run()).rejects.toThrow('7.44 cm');
  expect(installVerifiedMapZip).not.toHaveBeenCalled();
  expect(mapRepo.findByMowerSn(sn)).toEqual(before);
  expect(isFrameUnvalidated(sn)).toBe(false);
});

it.each(['runtime', 'unknown', 'rasters', 'late_files', 'late_rasters'])('keeps failed %s installs out of the DB and prevents generic apply even after restart', async kind => {
  const before = mapRepo.findByMowerSn(sn);
  deviceSettingsRepo.upsert(sn, PHOTO_DOCK_KEY, JSON.stringify(dock));
  if (kind === 'runtime') verifyRuntime.mockRejectedValueOnce(new Error('Runtime offset changed'));
  if (kind === 'late_files') verifyRuntime.mockImplementationOnce(async () => ({ ...installedSnapshot, csv_files: csv }));
  if (kind === 'late_rasters') verifyRuntime.mockImplementationOnce(async () => ({ ...installedSnapshot, map_files_b64: {} }));
  if (kind === 'unknown') vi.mocked(installVerifiedMapZip).mockImplementationOnce(async () => { markFrameUnvalidated(sn, { preservePhotoDock: true }); return null; });
  if (kind === 'rasters') vi.mocked(installVerifiedMapZip).mockImplementationOnce(async () => { markFrameUnvalidated(sn, { preservePhotoDock: true }); return { map_files_b64: {} }; });
  await expect(run()).rejects.toThrow();
  expect(mapRepo.findByMowerSn(sn)).toEqual(before);
  expect(getPhotoDockPose(sn)).toEqual(dock);
  expect(isFrameUnvalidated(sn)).toBe(true);
  loadFrameValidationFromDb();
  expect(isFrameUnvalidated(sn)).toBe(true);
  const calls = vi.mocked(installVerifiedMapZip).mock.calls.length;
  expect(await applyMapsToMower(sn)).toBe(false);
  expect(installVerifiedMapZip).toHaveBeenCalledTimes(calls);
});
