import type {
  EngineUpdate,
  WasmEngine,
  WasmModule,
  WorkerRequest,
  WorkerResponse,
  ComparisonRow,
  Algorithm,
  Board,
  SearchSettings,
} from './protocol';
import { algorithms } from './algorithms.ts';
import { summarize } from './comparison.ts';

const scope = self as unknown as DedicatedWorkerGlobalScope;
const post = (message: WorkerResponse) => scope.postMessage(message);
let engine: WasmEngine;
let wasm: WasmModule;
let runId = 0;
let running = false;
let paused = false;
let comparing = false;
let comparisonRows: ComparisonRow[] = [];
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

const yieldTask = () => new Promise<void>((resolve) => setTimeout(resolve, 0));
async function compare(id: number, config: Board & SearchSettings, repetitions: number) {
  const current = () => comparing && id === runId;
  const total = algorithms.length * (repetitions + 1);
  let completed = 0;
  const run = async (algorithm: Algorithm): Promise<EngineUpdate | null> => {
    let update = engine.start({
      ...config,
      algorithm,
      recordExpansions: false,
      includePath: false,
    });
    while (update.status === 'running') {
      await yieldTask();
      if (!current()) return null;
      update = engine.step(4096);
    }
    return update;
  };
  for (const { id: algorithm } of algorithms) {
    const samples: EngineUpdate[] = [];
    for (let pass = 0; pass <= repetitions; pass++) {
      await yieldTask();
      if (!current()) return;
      const result = await run(algorithm);
      if (!result || !current()) return;
      if (result.status === 'internal_error' || result.status === 'invalid_request')
        throw new Error('Configuration ou résultat invalide pendant la comparaison.');
      if (result.status === 'unsupported_configuration') {
        comparisonRows = [...comparisonRows, summarize(algorithm, [result])];
        completed += repetitions + 1;
      } else {
        completed++;
        if (pass > 0) samples.push(result);
        if (pass === repetitions)
          comparisonRows = [...comparisonRows, summarize(algorithm, samples)];
      }
      post({
        type: 'comparison-progress',
        runId: id,
        rows: comparisonRows,
        progress: { completed, total, algorithm },
      });
      if (result.status === 'unsupported_configuration') break;
    }
  }
  if (current()) {
    comparing = false;
    post({
      type: 'comparison-done',
      runId: id,
      rows: comparisonRows,
      complete: true,
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
  const module = await createModule({
    locateFile: (path) => new URL(path, moduleUrl).href,
  });
  wasm = module;
  engine = new module.Engine();
  post({ type: 'ready', engineVersion: module.engineVersion() });
  scope.onmessage = ({ data }: MessageEvent<WorkerRequest>) => {
    if (!data || !Number.isSafeInteger(data.runId) || data.runId < 1) return;
    if (data.type === 'start' || data.type === 'generate' || data.type === 'compare') {
      stopTimer();
      if (running || comparing) engine.cancel();
      running = false;
      comparing = false;
      runId = data.runId;
      if (data.type === 'generate') {
        try {
          const generated = wasm.generateMaze(data.config);
          if (generated.status !== 'success' || !generated.board)
            throw new Error('Invalid generation');
          post({ type: 'generated', runId, board: generated.board });
        } catch {
          post({
            type: 'operation-error',
            runId,
            message: 'La génération a échoué. Vérifiez les dimensions impaires et la graine.',
          });
        }
        return;
      }
      if (data.type === 'compare') {
        if (!Number.isInteger(data.repetitions) || data.repetitions < 3 || data.repetitions > 15) {
          post({
            type: 'operation-error',
            runId,
            message: 'Choisissez de 3 à 15 répétitions.',
          });
          return;
        }
        comparing = true;
        comparisonRows = [];
        const id = runId;
        compare(id, data.config, data.repetitions).catch(() => {
          if (id !== runId || !comparing) return;
          comparing = false;
          post({
            type: 'operation-error',
            runId: id,
            message:
              'La comparaison a échoué. Les résultats incomplets ne permettent pas de conclure.',
          });
        });
        return;
      }
      batchSize = Math.max(1, Math.min(4096, Math.floor(data.batchSize) || 1));
      paused = false;
      running = true;
      try {
        publish(engine.start(data.config));
      } catch {
        publish({
          status: 'invalid_request',
          message: 'Configuration de recherche invalide.',
        });
      }
      if (running) timer = setTimeout(() => tick(), 0);
      return;
    }
    if (data.runId !== runId) return;
    if (data.type === 'cancel' && comparing) {
      comparing = false;
      engine.cancel();
      post({
        type: 'comparison-done',
        runId,
        rows: comparisonRows,
        complete: false,
      });
      return;
    }
    if (!running) return;
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
