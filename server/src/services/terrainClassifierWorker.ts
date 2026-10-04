/**
 * Runs the SigLIP zero-shot pipeline off the main thread. onnxruntime-node's
 * run() is a synchronous native call, so on the main thread every crop held
 * the event loop for its whole duration and the API stopped answering (field
 * report, Oct 2026: 9-70 s stalls, one core at 100%). Protocol with
 * terrainClassifier.ts: posts {ready} or {loadError} once, then answers each
 * {id, jpeg} with {id, scores} or {id, error}.
 */
import { parentPort, workerData } from 'node:worker_threads';

interface Options { cacheDir: string; dtype: string; threads: number; labels: string[]; template: string }

async function main(): Promise<void> {
  const port = parentPort!;
  const { cacheDir, dtype, threads, labels, template } = workerData as Options;
  let classify: (jpeg: Uint8Array) => Promise<unknown>;
  try {
    const { pipeline, RawImage } = await import('@huggingface/transformers');
    const classifier = await pipeline('zero-shot-image-classification', 'Xenova/siglip-base-patch16-224', {
      cache_dir: cacheDir,
      dtype: dtype as 'q8' | 'fp32',
      ...(threads > 0 ? { session_options: { intraOpNumThreads: threads, interOpNumThreads: threads } } : {}),
    });
    classify = async (jpeg) => classifier(await RawImage.fromBlob(new Blob([jpeg], { type: 'image/jpeg' })), labels, { hypothesis_template: template });
  } catch (err) {
    port.postMessage({ loadError: err instanceof Error ? err.message : String(err) });
    return;
  }
  port.on('message', async ({ id, jpeg }: { id: number; jpeg: Uint8Array }) => {
    try {
      port.postMessage({ id, scores: await classify(jpeg) });
    } catch (err) {
      port.postMessage({ id, error: err instanceof Error ? err.message : String(err) });
    }
  });
  port.postMessage({ ready: true });
}

void main();
