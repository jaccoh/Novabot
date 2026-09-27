/**
 * End-to-end regression test — map rename survives mowing (#66).
 *
 * Reproduces waltervl's exact scenario against the REAL upload endpoint:
 *   1. Three work maps are renamed (map0→"achter", map1→"zij", map2→"testje").
 *   2. The mower mows and re-uploads its map ZIP (uploadEquipmentMap).
 *   3. The mower mows AGAIN and re-uploads a SECOND time.
 *
 * Both re-uploads must keep the aliases AND the stable canonical_name slots,
 * with no duplicate rows. The bug reset the alias to the default label on the
 * first re-upload (alias matched, but canonical_name was re-derived to NULL) and
 * then lost it entirely on the second (NULL canonical no longer matched). This
 * test runs the upload TWICE precisely because the single-upload "fix" passed
 * while the real repro (mow → mow) still failed.
 */
import { describe, it, expect, beforeEach, afterEach, vi } from 'vitest';
import request from 'supertest';
import express from 'express';
import { execSync } from 'node:child_process';
import fs from 'node:fs';
import os from 'node:os';
import path from 'node:path';

// map.ts reads STORAGE_PATH at import time — set it to a throwaway dir first.
const STORAGE = fs.mkdtempSync(path.join(os.tmpdir(), 'novabot-maptest-'));
process.env.STORAGE_PATH = STORAGE;

const { mapRouter } = await import('../../routes/map.js');
const { mapRepo, userRepo, equipmentRepo } = await import('../../../db/repositories/index.js');
const { signToken } = await import('../../../middleware/auth.js');
const { withMowerMapOperation } = await import('../../../services/mowerMapOperation.js');
const { markFrameUnvalidated, clearFrameUnvalidated } = await import('../../../services/frameValidation.js');

const SN = 'LFIN2231000367';
const SQUARE = '0,0\n2,0\n2,2\n0,2\n';

const app = express();
app.use(express.json());
app.use('/api/nova-file-server/map', mapRouter);

/** Build a real mower map ZIP (csv_file/ with 3 work maps + a unicom). */
function buildMapZip(): string {
  const dir = fs.mkdtempSync(path.join(os.tmpdir(), 'novabot-zip-'));
  const csvDir = path.join(dir, 'csv_file');
  fs.mkdirSync(csvDir);
  fs.writeFileSync(path.join(csvDir, 'map0_work.csv'), SQUARE);
  fs.writeFileSync(path.join(csvDir, 'map1_work.csv'), SQUARE);
  fs.writeFileSync(path.join(csvDir, 'map2_work.csv'), SQUARE);
  fs.writeFileSync(path.join(csvDir, 'map0tocharge_unicom.csv'), '0,0\n0,1\n');
  fs.writeFileSync(path.join(csvDir, 'map0_0_obstacle.csv'), SQUARE);
  fs.writeFileSync(path.join(csvDir, 'map_info.json'),
    JSON.stringify({ charging_pose: { x: 0, y: 0, orientation: 0 } }));
  const zipPath = path.join(dir, `${SN}.zip`);
  execSync(`cd "${dir}" && zip -q -r "${zipPath}" csv_file`);
  return zipPath;
}

function workAliasesBySlot(): Record<string, string | null> {
  const rows = mapRepo.findByMowerSn(SN).filter(r => r.map_type === 'work');
  const out: Record<string, string | null> = {};
  for (const r of rows) out[r.canonical_name ?? `?${r.map_id}`] = r.map_name;
  return out;
}

async function reuploadFromMower(): Promise<void> {
  const zip = buildMapZip();
  const res = await request(app)
    .post('/api/nova-file-server/map/uploadEquipmentMap')
    .field('sn', SN)
    .attach('file', zip, `${SN}.zip`);
  expect(res.status).toBe(200);
}

describe('map rename survives repeated mowing / re-upload (#66)', () => {
  beforeEach(() => {
    clearFrameUnvalidated(SN);
    // Start from the renamed state the user reported.
    mapRepo.deleteByMowerSn?.(SN);
    const seed = (slot: string, alias: string) => mapRepo.upsert({
      map_id: `${slot}-id`, mower_sn: SN, map_name: alias, canonical_name: slot,
      map_area: JSON.stringify([{ x: 0, y: 0 }, { x: 2, y: 0 }, { x: 2, y: 2 }, { x: 0, y: 2 }]),
      map_max_min: null, file_name: `${slot}_work.csv`, file_size: 0, map_type: 'work',
    });
    seed('map0', 'achter');
    seed('map1', 'zij');
    seed('map2', 'testje');
    mapRepo.upsert({
      map_id: 'obstacle-id', mower_sn: SN, map_name: 'motherInLaw', canonical_name: 'map0_0_obstacle',
      map_area: null, map_max_min: null, file_name: 'map0_0_obstacle.csv', file_size: 0, map_type: 'obstacle',
    });
  });
  afterEach(() => { clearFrameUnvalidated(SN); vi.restoreAllMocks(); });

  it('keeps aliases + canonical slots after the FIRST re-upload', async () => {
    await reuploadFromMower();
    expect(workAliasesBySlot()).toEqual({ map0: 'achter', map1: 'zij', map2: 'testje' });
    const obstacles = mapRepo.findByMowerSn(SN).filter(r => r.map_type === 'obstacle');
    expect(obstacles.map(r => [r.map_id, r.map_name])).toEqual([['obstacle-id', 'motherInLaw']]);
  });

  it('keeps them after a SECOND re-upload (the real repro — mow twice)', async () => {
    await reuploadFromMower();
    await reuploadFromMower();
    expect(workAliasesBySlot()).toEqual({ map0: 'achter', map1: 'zij', map2: 'testje' });
    // No duplicate work rows crept in.
    const workRows = mapRepo.findByMowerSn(SN).filter(r => r.map_type === 'work');
    expect(workRows).toHaveLength(3);
  });

  it.each(['busy', 'unvalidated'])('rejects passive uploads and app mutations while %s without changing rows or the latest ZIP', async state => {
    userRepo.create('map-owner', 'map-owner@example.test', 'unused', 'Map owner');
    equipmentRepo.create({ equipment_id: 'map-equipment', user_id: 'map-owner', mower_sn: SN, charger_sn: null });
    const token = signToken({ userId: 'map-owner', email: 'map-owner@example.test' });
    const latest = path.join(STORAGE, 'maps', `${SN}_latest.zip`);
    fs.writeFileSync(latest, 'previous verified ZIP');
    const before = mapRepo.findByMowerSn(SN);
    const files = fs.readdirSync(path.dirname(latest)).sort();
    const check = async () => {
      const native = await request(app).post('/api/nova-file-server/map/uploadEquipmentMap')
        .field('sn', SN).attach('file', buildMapZip(), `${SN}.zip`);
      expect(native.status).toBe(409);
      const fragment = await request(app).post('/api/nova-file-server/map/fragmentUploadEquipmentMap')
        .set('Authorization', token).field('sn', SN).field('uploadId', 'blocked-upload')
        .field('mapName', 'map4').attach('file', Buffer.from(SQUARE), 'map4_work.csv');
      expect(fragment.status).toBe(409);
      const alias = await request(app).post('/api/nova-file-server/map/updateEquipmentMapAlias')
        .set('Authorization', token).send({ mapId: 'map0-id', sn: 'unlocked-other-mower', mapName: 'Changed' });
      expect(alias.status).toBe(409);
      expect(mapRepo.findByMowerSn(SN)).toEqual(before);
      expect(fs.readFileSync(latest, 'utf8')).toBe('previous verified ZIP');
      expect(fs.readdirSync(path.dirname(latest)).sort()).toEqual(files);
    };
    if (state === 'busy') await withMowerMapOperation(SN, check);
    else { markFrameUnvalidated(SN); await check(); }
  });

  it('rechecks the frame after asynchronous fragment assembly before inserting a map', async () => {
    userRepo.create('map-owner', 'map-owner@example.test', 'unused', 'Map owner');
    equipmentRepo.create({ equipment_id: 'map-equipment', user_id: 'map-owner', mower_sn: SN, charger_sn: null });
    const token = signToken({ userId: 'map-owner', email: 'map-owner@example.test' });
    const before = mapRepo.findByMowerSn(SN);
    const createStream = fs.createWriteStream;
    let assembling = false;
    vi.spyOn(fs, 'createWriteStream').mockImplementation((...args) => {
      const stream = createStream(...args);
      if (String(args[0]).endsWith('finishing-upload.bin')) {
        assembling = true;
        stream.prependOnceListener('finish', () => markFrameUnvalidated(SN));
      }
      return stream;
    });
    const res = await request(app).post('/api/nova-file-server/map/fragmentUploadEquipmentMap')
      .set('Authorization', token).field('sn', SN).field('uploadId', 'finishing-upload')
      .field('mapName', 'map4').field('chunkIndex', '0').field('chunksTotal', '1')
      .attach('file', Buffer.from(SQUARE), 'map4_work.csv');
    expect(res.status).toBe(409);
    expect(assembling).toBe(true);
    expect(mapRepo.findByMowerSn(SN)).toEqual(before);
    expect(fs.readdirSync(path.join(STORAGE, 'maps')).some(n => n.startsWith('finishing-upload'))).toBe(false);
  });
});
