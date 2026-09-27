import { mkdirSync, writeFileSync, renameSync } from 'node:fs';
import path from 'node:path';
import { db } from '../db/database.js';
import { mapRepo, deviceSettingsRepo } from '../db/repositories/index.js';
import { polygonArea } from '../maps/editGeometry.js';
import { validateMapRasters } from '../maps/validateGrid.js';
import { isDeviceOnline } from '../mqtt/broker.js';
import { csvZip, type ConfirmedCopyDocks } from './dockChannelRepair.js';
import { getPhotoDockPose, PHOTO_DOCK_KEY } from './dockPhotoReference.js';
import { clearFrameUnvalidated, isFrameUnvalidated, markFrameUnvalidated } from './frameValidation.js';
import { beginMapApply } from './mapApplyStatus.js';
import { assertMowerMapOperation } from './mowerMapOperation.js';
import { installVerifiedMapZip } from './mowerMapApply.js';
import { stablePosition } from './positionTelemetry.js';
import { persistZoneCopy, type CopyPlan, type PersistResult } from './zoneCopy.js';

/** Both leases remain owned by the copy route until device verification and DB commit. */
export async function installZoneCopy(
  sn: string,
  plan: CopyPlan,
  opts: { alias: string | null; acceptChannel: boolean },
  docks: ConfirmedCopyDocks,
  verifyBeforeInstall: () => Promise<{ verifyRuntime: () => Promise<Record<string, unknown>> }>,
): Promise<PersistResult> {
  const { targetOperation: operation, sourceOperation, targetSnapshot: before, target: dock } = docks;
  const ready = () => {
    assertMowerMapOperation(sn, operation);
    assertMowerMapOperation(sourceOperation.sn, sourceOperation);
    const position = stablePosition(sn, { docked: true });
    if (!isDeviceOnline(sn) || !position || Math.hypot(position.x - dock.x, position.y - dock.y) > .05) {
      throw new Error('Zet de doelmaaier op zijn eigen dock met stabiele RTK Fixed-lokalisatie binnen 5 cm van de opgeslagen dockpositie.');
    }
  };
  ready();
  if (!plan.ok || isFrameUnvalidated(sn)) throw new Error('De kopie of het kaartframe is niet bevestigd.');
  const original = before.csv_files as Record<string, string>;
  const x3 = before.x3_csv_files as Record<string, string>;
  if (!original || !x3 || Object.keys(original).length !== Object.keys(x3).length ||
    Object.entries(original).some(([name, value]) => typeof value !== 'string' || x3[name] !== value)) {
    throw new Error('De twee kaartkopieën op de doelmaaier verschillen; synchroniseer die eerst.');
  }
  const csv = { ...original };
  const workName = `${plan.canonical}_work.csv`;
  if (workName in csv) throw new Error('Het gekozen kaartslot is niet meer vrij.');
  const additions = [{ canonical: `${plan.canonical}_work`, points: plan.work }, ...plan.obstacles,
    ...(opts.acceptChannel ? plan.channels : [])];
  for (const area of additions) csv[`${area.canonical}.csv`] = area.points.map(p => `${p.x.toFixed(6)},${p.y.toFixed(6)}`).join('\n') + '\n';
  const metadata = JSON.parse(csv['map_info.json']);
  metadata[workName] = { map_size: Math.round(polygonArea(plan.work) * 100) / 100 };
  csv['map_info.json'] = JSON.stringify(metadata, null, 3) + '\n';

  const rows = mapRepo.findByMowerSn(sn), photo = getPhotoDockPose(sn);
  const unchanged = () => JSON.stringify(mapRepo.findByMowerSn(sn)) === JSON.stringify(rows);
  const root = path.resolve(process.env.STORAGE_PATH ?? './storage');
  const backupDir = path.join(root, 'zone-copy', operation.id);
  mkdirSync(backupDir, { recursive: true });
  writeFileSync(path.join(backupDir, 'before.json'), JSON.stringify({ sn, sourceSn: sourceOperation.sn, snapshot: before,
    sourceSnapshot: docks.sourceSnapshot, rows, calibration: mapRepo.getCalibration(sn), photo, plan }), { flag: 'wx' });
  const bytes = await csvZip(csv);
  writeFileSync(path.join(backupDir, 'copy.zip'), bytes, { flag: 'wx' });
  // Do the fresh comparison after ZIP preparation, immediately before any device write.
  const verification = await verifyBeforeInstall();
  ready();
  if (!unchanged()) throw new Error('De doelkaart is tijdens de voorbereiding gewijzigd.');
  const apply = beginMapApply(sn);
  apply.phase('syncing');
  let transferFailed = false, installed = false;
  try {
    const after = await installVerifiedMapZip(sn, { bytes, expectedCsv: new Map(Object.entries(csv)), before, anchor: dock }, operation, apply);
    if (!after) { transferFailed = true; throw new Error('Kaartoverdracht niet bevestigd; de kopie is niet opgeslagen.'); }
    installed = true;
    const rasters = after.map_files_b64 as Record<string, string>;
    const text = after.map_files_text as Record<string, string>;
    const slots = Object.keys(csv).filter(n => /^map\d+_work\.csv$/.test(n)).map(n => n.slice(0, -9));
    const validation = validateMapRasters(rasters);
    if (!rasters || !text || !validation.ok || ['map', ...slots].some(n =>
      !rasters[`${n}.pgm`] || !text[`${n}.yaml`] || !validation.stats[`${n}.pgm`]?.total)) {
      throw new Error('De navigatiekaarten zijn niet geldig opgebouwd.');
    }
    const current = await verification.verifyRuntime();
    // The runtime check takes time; its final native read must still contain this exact installation.
    if (current.pos_json !== before.pos_json || current.charging_station_yaml !== before.charging_station_yaml ||
      ['csv_files', 'x3_csv_files'].some(key => {
        const files = current[key] as Record<string, string> | undefined;
        return !files || Object.keys(files).length !== Object.keys(csv).length || Object.entries(csv).some(([name, text]) => files[name] !== text);
      })) throw new Error('De kaartbestanden zijn tijdens de eindcontrole gewijzigd.');
    if (['map_files_b64', 'map_files_text'].some(key => {
      const previous = after[key] as Record<string, string>;
      const files = current[key] as Record<string, string> | undefined;
      return !files || Object.keys(files).length !== Object.keys(previous).length || Object.entries(previous).some(([name, text]) => files[name] !== text);
    })) throw new Error('De navigatiekaarten zijn tijdens de eindcontrole gewijzigd.');
    ready();
    if (!unchanged()) throw new Error('De doelkaart is tijdens de overdracht gewijzigd.');
    writeFileSync(path.join(backupDir, 'after.json'), JSON.stringify(after), { flag: 'wx' });
    const latest = path.join(root, 'maps', `${sn}_latest.zip`);
    mkdirSync(path.dirname(latest), { recursive: true });
    writeFileSync(`${latest}.copy`, bytes);
    renameSync(`${latest}.copy`, latest);
    const saved = db.transaction(() => {
      const result = persistZoneCopy(sn, plan, { ...opts, dockOrientation: dock.orientation });
      if (photo) deviceSettingsRepo.upsert(sn, PHOTO_DOCK_KEY, JSON.stringify(photo));
      return result;
    })();
    clearFrameUnvalidated(sn);
    apply.done();
    return saved;
  } catch (error) {
    if (installed && !isFrameUnvalidated(sn)) markFrameUnvalidated(sn, { preservePhotoDock: true });
    if (!transferFailed) apply.fail('sync_failed'); // Preserve the installer's specific failure.
    throw error;
  }
}
