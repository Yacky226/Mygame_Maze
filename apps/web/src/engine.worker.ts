import type {
  EngineUpdate,
  WasmEngine,
  WasmModule,
  WorkerRequest,
  WorkerResponse,
} from './protocol';

const scope = self as unknown as DedicatedWorkerGlobalScope;
const post = (message: WorkerResponse) => scope.postMessage(message);
let engine: WasmEngine;
let runId = 0;
let running = false;
let paused = false;
let batchSize = 16;
let timer: ReturnType<typeof setTimeout> | undefined;

function stopTimer() {
  clearTimeout(timer);
  timer = undefined;
}
function publish(update: EngineUpdate) {
  if (update.status !== 'running') {
    running = false;
    stopTimer();
  }
  post({ type: 'update', runId, update });
}
function tick(single = false) {
  timer = undefined;
  if (!running || (paused && !single)) return;
  try {
    publish(engine.step(single ? 1 : batchSize));
    // A task boundary lets pause/cancel/start messages run before the next batch.
    if (running && !paused) timer = setTimeout(() => tick(), 24);
  } catch {
    running = false;
    post({
      type: 'fatal',
      message: 'Le moteur a interrompu la recherche. Rechargez l’atelier pour réessayer.',
    });
  }
}

async function initialize() {
  const moduleUrl = new URL(
    `${import.meta.env.BASE_URL}engine/maze_engine.mjs`,
    scope.location.origin,
  ).href;
  const { default: createModule } = (await import(/* @vite-ignore */ moduleUrl)) as {
    default(options: { locateFile(path: string): string }): Promise<WasmModule>;
  };
  const module = await createModule({ locateFile: (path) => new URL(path, moduleUrl).href });
  engine = new module.Engine();
  post({ type: 'ready', engineVersion: module.engineVersion() });
  scope.onmessage = ({ data }: MessageEvent<WorkerRequest>) => {
    if (!data || !Number.isSafeInteger(data.runId) || data.runId < 1) return;
    if (data.type === 'start') {
      stopTimer();
      runId = data.runId;
      batchSize = Math.max(1, Math.min(4096, Math.floor(data.batchSize) || 1));
      paused = false;
      running = true;
      try {
        publish(engine.start(data.config));
      } catch {
        publish({ status: 'invalid_request', message: 'Configuration de recherche invalide.' });
      }
      if (running) timer = setTimeout(() => tick(), 0);
      return;
    }
    if (data.runId !== runId || !running) return;
    if (data.type === 'pause') {
      paused = true;
      stopTimer();
      post({ type: 'paused', runId });
    } else if (data.type === 'resume') {
      paused = false;
      stopTimer();
      post({ type: 'resumed', runId });
      timer = setTimeout(() => tick(), 0);
    } else if (data.type === 'step' && paused) {
      tick(true);
    } else if (data.type === 'cancel') {
      stopTimer();
      publish(engine.cancel());
    } else if (data.type === 'speed') {
      batchSize = Math.max(1, Math.min(4096, Math.floor(data.batchSize) || 1));
    }
  };
}
initialize().catch(() =>
  post({
    type: 'fatal',
    message:
      'Impossible de charger le moteur. Vérifiez la construction WebAssembly et rechargez la page.',
  }),
);
