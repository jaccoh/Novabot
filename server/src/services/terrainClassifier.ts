/**
 * Zero-shot terrain-object classifier — objectherkenning-plan Task 6.
 *
 * Classificeert een RGB-crop (JPEG) van een terrain-cluster tegen een vaste
 * labellijst via SigLIP zero-shot-image-classification (`@huggingface/
 * transformers`, v4). Het model wordt ON-DEMAND gedownload naar
 * `STORAGE_PATH/models` — GEEN model in de Docker-image of in git.
 *
 * Testbaarheid: het pipeline-object is injecteerbaar via
 * `_setPipelineForTest()` zodat de test-suite NOOIT een model downloadt.
 */
import fs from 'node:fs';
import os from 'node:os';
import path from 'node:path';
import { Worker } from 'node:worker_threads';

/** EN prompt → NL naam → GLB-bestand (null = geen model, blijft voxels). */
export const LABELS: Array<{ prompt: string; nl: string; glb: string | null }> = [
  { prompt: 'trampoline', nl: 'Trampoline', glb: 'trampoline.glb' },
  { prompt: 'tree', nl: 'Boom', glb: 'tree.glb' },
  { prompt: 'bush', nl: 'Struik', glb: 'bush.glb' },
  { prompt: 'hydrangea', nl: 'Hortensia', glb: 'bush.glb' },
  { prompt: 'garden chair', nl: 'Tuinstoel', glb: 'chair.glb' },
  { prompt: 'garden table', nl: 'Tuintafel', glb: 'table.glb' },
  { prompt: 'flower pot with plant', nl: 'Bloempot', glb: 'flowerpot.glb' },
  { prompt: 'wooden barrel', nl: 'Houten ton', glb: 'barrel.glb' },
  { prompt: 'parasol', nl: 'Parasol', glb: 'parasol.glb' },
  { prompt: 'playground equipment', nl: 'Speeltoestel', glb: 'playset.glb' },
  { prompt: 'fence', nl: 'Schutting', glb: null },
  { prompt: 'charging station', nl: 'Laadstation', glb: null },
  { prompt: 'swimming pool', nl: 'Zwembad', glb: 'pool.glb' },
  { prompt: 'ball', nl: 'Bal', glb: 'ball.glb' },
];

/**
 * Achtergrond-vangers: doen mee als kandidaat zodat "geen object" ergens
 * heen kan, maar een top-1 hierop betekent gewoon `null` (blijft voxels).
 * Staan bewust NIET in LABELS — geen override-optie, geen GLB.
 */
export const SINK_PROMPTS = ['lawn'] as const;

/**
 * Q8 in plaats van fp32. Hetzelfde model, gekwantiseerd: `model_quantized.onnx`
 * is 201 MB waar `model.onnx` er 813 is. Op een server die de herkenning naast
 * van alles draait is dat het verschil tussen passen en de machine de swap in
 * trekken (live .247, 2026-09-14: fp32 laden bij 137 MB vrij legde de hele NAS
 * plat). `TERRAIN_MODEL_DTYPE=fp32` zet het terug voor wie geheugen over heeft;
 * de drempels hieronder volgen die keuze. Fp16 is geen optie: onnxruntime-node
 * struikelt op CPU over de fusion in dat model (getest 2026-09-14).
 */
export const MODEL_DTYPE = process.env.TERRAIN_MODEL_DTYPE ?? 'q8';

/**
 * SigLIP scoort met sigmoids (niet softmax): absolute scores blijven laag,
 * zelfs bij een overduidelijke winnaar (praktijkmeting 2026-07-20: struik
 * 0.31, nummer 2 op 0.003). Daarom een marge-regel i.p.v. een hoge kale
 * drempel: top-1 moet minimaal CONFIDENCE_MIN scoren ÉN MARGIN_RATIO keer
 * boven de nummer 2 zitten.
 *
 * De drempels horen bij de precisie van het model. Q8 kiest hetzelfde label
 * als fp32 maar scoort er ongeveer twee tot drie keer lager op, dus de
 * fp32-drempels zouden er een derde van de vondsten door de vingers laten
 * glippen. Nagemeten op de 86 crops van LFIN2230700238 met de fp32-uitspraken
 * uit de database als referentie (2026-09-14):
 *
 *   drempels          gevonden   vals positief
 *   0.12 / 4  (fp32)   9 van 16       0
 *   0.10 / 2  (q8)    11 van 16       0
 *   0.08 / 1.5        14 van 16       1
 *
 * 0.10 / 2 is het laatste punt zonder vals positief. De vijf die q8 mist zijn
 * allemaal struiken die hij wél als struik bovenaan zet, maar te zwak.
 */
const THRESHOLDS: Record<string, { confidence: number; margin: number }> = {
  fp32: { confidence: 0.12, margin: 4 },
  q8: { confidence: 0.10, margin: 2 },
};

const ACTIVE = THRESHOLDS[MODEL_DTYPE] ?? THRESHOLDS.fp32;
export const CONFIDENCE_MIN = ACTIVE.confidence;
export const MARGIN_RATIO = ACTIVE.margin;

/** SigLIP is getraind met dit prompt-sjabloon; zonder blijven scores ~3x lager. */
export const PROMPT_TEMPLATE = 'a photo of a {}';

/** Eén classificatie-run over alle LABELS voor één crop. */
type PipelineFn = (jpeg: Buffer) => Promise<Array<{ label: string; score: number }>>;

/**
 * Hoe lang het model in het geheugen blijft nadat de laatste crop is
 * geclassificeerd. `0` = nooit lossen. Kort gehouden omdat de uploads tijdens
 * een maaibeurt om de paar minuten binnenkomen: bij vijf minuten stond de klok
 * altijd weer terug en ging het model in de praktijk nooit meer weg.
 */
export const IDLE_UNLOAD_MS = Number(process.env.TERRAIN_MODEL_IDLE_MS ?? 90_000);

/**
 * Ondergrens vrij werkgeheugen waaronder het model NIET geladen wordt. De
 * herkenning is een bijzaak; hem laten laden op een machine die al niets meer
 * over heeft kost de maaier zijn verbinding met de server. Onder de grens
 * slaat de batch over en probeert de volgende sessie het gewoon opnieuw.
 */
export const MIN_FREE_MB = Number(process.env.TERRAIN_MIN_FREE_MB ?? 700);

/**
 * Hoeveel cores de herkenning mag pakken. Onnxruntime neemt er standaard
 * zoveel als er zijn; op een machine met vier cores betekende dat 100% CPU en
 * een server die geen HTTP meer beantwoordde, ook nadat het geheugenprobleem
 * met q8 was opgelost (live .247, 2026-09-14). De herkenning is een bijzaak en
 * mag niet de hele machine claimen. `0` = laat onnxruntime zelf kiezen.
 */
export const MODEL_THREADS = Number(process.env.TERRAIN_MODEL_THREADS ?? 1);

/**
 * Mag het model geladen worden bij dit vrije geheugen? Onbekend (geen meting)
 * = ja, want een ontbrekende meting is geen reden om de functie uit te zetten.
 */
export function memoryAllowsLoad(freeMb: number | null, minFreeMb: number = MIN_FREE_MB): boolean {
  return freeMb === null || freeMb >= minFreeMb;
}

/**
 * Vrij geheugen van de HOST in MB. In een container toont /proc/meminfo de
 * host, en dat is precies wat telt: het is de host die gaat swappen.
 * `MemAvailable` (niet `MemFree`) is de kernel-schatting van wat een proces
 * echt kan krijgen zonder te swappen. Geen procfs (macOS) → os.freemem().
 */
export function availableMemoryMb(meminfo?: string): number | null {
  try {
    const raw = meminfo ?? fs.readFileSync('/proc/meminfo', 'utf8');
    const m = raw.match(/^MemAvailable:\s+(\d+) kB$/m);
    if (m) return Math.floor(Number(m[1]) / 1024);
  } catch {
    // geen procfs: val terug op de node-meting hieronder
  }
  if (meminfo !== undefined) return null;   // test gaf expliciet iets mee
  const free = os.freemem();
  return Number.isFinite(free) && free > 0 ? Math.floor(free / 1024 / 1024) : null;
}

let currentPipeline: PipelineFn | null = null;
let disposeCurrent: (() => Promise<unknown>) | null = null;
let idleTimer: ReturnType<typeof setTimeout> | null = null;
let loading: Promise<boolean> | null = null;
/** Aantal crops dat nú in het model zit — nooit lossen terwijl dit > 0 is. */
let inFlight = 0;

/** (Her)start de inactiviteitsklok. Geen pipeline of klok uit = niets doen. */
function touchIdleTimer(): void {
  if (idleTimer) clearTimeout(idleTimer);
  idleTimer = null;
  if (!currentPipeline || !(IDLE_UNLOAD_MS > 0)) return;
  idleTimer = setTimeout(() => void unloadClassifier(), IDLE_UNLOAD_MS);
  idleTimer.unref?.();
}

/**
 * Haalt het model uit het geheugen en geeft de onnxruntime-sessie vrij.
 * Idempotent. Loopt er nog een classificatie, dan wordt het lossen uitgesteld
 * tot na de inactiviteitsklok. De eerstvolgende `initClassifier()` laadt het
 * model gewoon opnieuw — de gewichten staan al op schijf, dus dat is een
 * lokale load, geen download.
 */
export async function unloadClassifier(): Promise<void> {
  if (idleTimer) clearTimeout(idleTimer);
  idleTimer = null;
  if (inFlight > 0) {
    touchIdleTimer();
    return;
  }
  const dispose = disposeCurrent;
  currentPipeline = null;
  disposeCurrent = null;
  if (!dispose) return;
  try {
    await dispose();
    console.log('[terrainClassifier] model uit het geheugen gehaald na inactiviteit');
  } catch (err) {
    console.warn(
      '[terrainClassifier] vrijgeven van het model faalde:',
      err instanceof Error ? err.message : err,
    );
  }
}

// The model runs in terrainClassifierWorker; under tsx (dev) that is the .ts
// source, which the worker needs tsx to load.
const OWN_SOURCE_IS_TS = import.meta.url.endsWith('.ts');
const DEFAULT_WORKER = new URL(`./terrainClassifierWorker.${OWN_SOURCE_IS_TS ? 'ts' : 'js'}`, import.meta.url);
let workerUrl: URL = DEFAULT_WORKER;
let workerExtra: Record<string, unknown> = {};

/** Test-only: a stand-in worker script (`null` restores the real one). */
export function _setWorkerUrlForTest(url: URL | null, extra: Record<string, unknown> = {}): void {
  workerUrl = url ?? DEFAULT_WORKER;
  workerExtra = extra;
}

/** Test-only: injecteert (of verwijdert, met `null`) de pipeline. */
export function _setPipelineForTest(
  fn: PipelineFn | null,
  dispose?: () => Promise<unknown>,
): void {
  if (idleTimer) clearTimeout(idleTimer);
  idleTimer = null;
  currentPipeline = fn;
  disposeCurrent = fn ? dispose ?? null : null;
  loading = null;
  inFlight = 0;
}

/**
 * Laadt lazily de SigLIP zero-shot pipeline (on-demand download naar
 * `STORAGE_PATH/models`). Idempotent: eenmaal geladen wordt de pipeline
 * hergebruikt. `TERRAIN_CLASSIFY=0` schakelt de feature volledig uit.
 * Download-/laadfouten geven `false` + een warning terug — de aanroepende
 * batch slaat dan deze sessie over en probeert het de volgende sessie
 * opnieuw (er wordt niets blijvend als "mislukt" onthouden).
 */
export async function initClassifier(): Promise<boolean> {
  if (process.env.TERRAIN_CLASSIFY === '0') {
    return false;
  }
  if (currentPipeline) {
    touchIdleTimer();
    return true;
  }
  // Geheugenpoort vóór de laadpoging, niet erin: een te volle machine is geen
  // mislukte download en mag niet in `loading` blijven hangen.
  const freeMb = availableMemoryMb();
  if (!memoryAllowsLoad(freeMb)) {
    console.warn(
      `[terrainClassifier] ${freeMb} MB vrij, onder de ondergrens van ${MIN_FREE_MB} MB — `
      + 'model niet geladen, batch wordt overgeslagen',
    );
    return false;
  }
  // Eén laadpoging tegelijk: twee maaiers die tegelijk binnenkomen mogen niet
  // allebei hun eigen kopie van het model inladen.
  loading ??= loadPipeline().finally(() => {
    loading = null;
  });
  return loading;
}

/**
 * Starts the worker that loads and runs the model. The main thread only posts
 * JPEGs and awaits scores, so inference never blocks HTTP or MQTT handling;
 * with MODEL_THREADS = 1 it costs at most one core. Unloading terminates the
 * worker, which frees the whole model.
 */
async function loadPipeline(): Promise<boolean> {
  let worker: Worker;
  try {
    worker = new Worker(workerUrl, {
      execArgv: workerUrl.pathname.endsWith('.ts') ? ['--import', 'tsx'] : undefined,
      workerData: {
        cacheDir: path.resolve(process.env.STORAGE_PATH ?? './storage', 'models'),
        dtype: MODEL_DTYPE,
        threads: MODEL_THREADS,
        labels: [...LABELS.map((l) => l.prompt), ...SINK_PROMPTS],
        template: PROMPT_TEMPLATE,
        ...workerExtra,
      },
    });
  } catch (err) {
    console.warn('[terrainClassifier] kon de herkenningsworker niet starten:', err instanceof Error ? err.message : err);
    return false;
  }
  const loadError = await new Promise<string | null>((resolve) => {
    // Only our own listeners come off again: Worker.removeAllListeners()
    // also drops Node's internal message wiring.
    const done = (result: string | null) => {
      worker.off('message', onMessage); worker.off('error', onError); worker.off('exit', onExit);
      resolve(result);
    };
    const onMessage = (m: { ready?: boolean; loadError?: string }) => done(m?.ready ? null : String(m?.loadError ?? 'onbekend'));
    const onError = (err: Error) => done(err.message);
    const onExit = (code: number) => done(`worker gestopt (${code})`);
    worker.on('message', onMessage); worker.on('error', onError); worker.on('exit', onExit);
  });
  if (loadError) {
    console.warn(
      '[terrainClassifier] kon SigLIP-model niet laden/downloaden — batch wordt overgeslagen, volgende sessie opnieuw geprobeerd:',
      loadError,
    );
    void worker.terminate();
    return false;
  }

  const waiting = new Map<number, { resolve: (s: Array<{ label: string; score: number }>) => void; reject: (e: Error) => void }>();
  let nextId = 0;
  const pipe: PipelineFn = (jpeg) => new Promise((resolve, reject) => {
    const id = nextId++;
    waiting.set(id, { resolve, reject });
    worker.postMessage({ id, jpeg });
  });
  worker.on('message', (m: { id: number; scores?: Array<{ label: string; score: number }>; error?: string }) => {
    const w = waiting.get(m.id);
    if (!w) return;
    waiting.delete(m.id);
    if (m.error !== undefined) w.reject(new Error(m.error));
    else w.resolve(m.scores ?? []);
  });
  worker.on('error', (err) => console.warn('[terrainClassifier] herkenningsworker faalde:', err.message));
  worker.on('exit', () => {
    for (const w of waiting.values()) w.reject(new Error('recognition worker stopped'));
    waiting.clear();
    // A crash drops the pipeline; the next initClassifier() starts a new worker.
    if (currentPipeline === pipe) { currentPipeline = null; disposeCurrent = null; }
  });
  currentPipeline = pipe;
  disposeCurrent = () => worker.terminate();
  touchIdleTimer();
  return true;
}

/**
 * Classificeert één crop. Retourneert `null` als er (nog) geen pipeline
 * beschikbaar is, als de top-1 een achtergrond-vanger is (`SINK_PROMPTS`),
 * of als de marge-regel faalt (top-1 < CONFIDENCE_MIN of niet MARGIN_RATIO
 * keer boven de nummer 2).
 */
export async function classifyCrop(
  jpeg: Buffer,
): Promise<{ className: string; nl: string; confidence: number } | null> {
  const pipe = currentPipeline;
  if (!pipe) {
    return null;
  }
  inFlight++;
  try {
    const scores = await pipe(jpeg);
    const sorted = [...scores].sort((a, b) => b.score - a.score);
    const best = sorted[0];
    const second = sorted[1];
    if (!best || best.score < CONFIDENCE_MIN) {
      return null;
    }
    if (second && best.score < MARGIN_RATIO * second.score) {
      return null;
    }
    if ((SINK_PROMPTS as readonly string[]).includes(best.label)) {
      return null;
    }
    const label = LABELS.find((l) => l.prompt === best.label);
    if (!label) {
      return null;
    }
    return { className: label.prompt, nl: label.nl, confidence: best.score };
  } catch (err) {
    console.warn('[CLASSIFY] crop-classificatie faalde (corrupte jpeg?):', err);
    return null;
  } finally {
    inFlight--;
    touchIdleTimer();
  }
}
