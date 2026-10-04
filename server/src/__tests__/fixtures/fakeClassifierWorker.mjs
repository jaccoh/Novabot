// Stands in for terrainClassifierWorker in tests: same messages, no model.
// It busy-waits like a synchronous onnxruntime run, to prove the caller's
// event loop keeps running meanwhile.
import { parentPort, workerData } from 'node:worker_threads';

if (workerData.fail) {
  parentPort.postMessage({ loadError: 'no model' });
} else {
  parentPort.on('message', ({ id, jpeg }) => {
    if (jpeg[0] === 0xde) process.exit(3); // simulated crash
    const until = Date.now() + 200;
    while (Date.now() < until) { /* hold this thread, not the caller's */ }
    parentPort.postMessage({ id, scores: workerData.labels.map(label => ({ label, score: label === 'bush' ? 0.31 : 0.001 })) });
  });
  parentPort.postMessage({ ready: true });
}
