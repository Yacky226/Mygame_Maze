import { useCallback, useEffect, useRef, useState } from 'react';
import type {
  Board,
  EngineUpdate,
  SearchSettings,
  WorkerRequest,
  WorkerResponse,
  GenerationSettings,
  ComparisonReport,
  ComparisonProgress,
} from './protocol';
import { makeReport } from './comparison.ts';

export function useSearch() {
  const worker = useRef<Worker | null>(null);
  const runId = useRef(0);
  const started = useRef(0);
  const [ready, setReady] = useState(false);
  const [version, setVersion] = useState('');
  const [error, setError] = useState('');
  const [result, setResult] = useState<EngineUpdate | null>(null);
  const [expanded, setExpanded] = useState<number[]>([]);
  const [paused, setPaused] = useState(false);
  const [elapsed, setElapsed] = useState(0);
  const [generating, setGenerating] = useState(false);
  const [comparing, setComparing] = useState(false);
  const [comparison, setComparison] = useState<ComparisonReport | null>(null);
  const [comparisonProgress, setComparisonProgress] = useState<ComparisonProgress | null>(null);
  const [comparisonError, setComparisonError] = useState('');
  const pendingGeneration = useRef<{
    resolve(board: Board): void;
    reject(error: Error): void;
  } | null>(null);
  const active = result?.status === 'running';

  useEffect(() => {
    const instance = new Worker(new URL('./engine.worker.ts', import.meta.url), { type: 'module' });
    worker.current = instance;
    instance.onmessage = ({ data }: MessageEvent<WorkerResponse>) => {
      if (data.type === 'ready') {
        setReady(true);
        setVersion(data.engineVersion);
        return;
      }
      if (data.type === 'fatal') {
        setError(data.message);
        setReady(false);
        setResult(null);
        setPaused(false);
        setGenerating(false);
        setComparing(false);
        pendingGeneration.current?.reject(new Error(data.message));
        pendingGeneration.current = null;
        return;
      }
      if (data.runId !== runId.current) return;
      if (data.type === 'generated') {
        setGenerating(false);
        pendingGeneration.current?.resolve(data.board);
        pendingGeneration.current = null;
        return;
      }
      if (data.type === 'operation-error') {
        setGenerating(false);
        setComparing(false);
        setComparisonError(data.message);
        pendingGeneration.current?.reject(new Error(data.message));
        pendingGeneration.current = null;
        return;
      }
      if (data.type === 'comparison-progress') {
        setComparisonProgress(data.progress);
        setComparison((previous) => (previous ? { ...previous, rows: data.rows } : null));
        return;
      }
      if (data.type === 'comparison-done') {
        setComparing(false);
        setComparisonProgress(null);
        setComparison((previous) =>
          previous ? { ...previous, rows: data.rows, complete: data.complete } : null,
        );
        return;
      }
      if (data.type === 'paused') setPaused(true);
      else if (data.type === 'resumed') setPaused(false);
      else if (data.type === 'update') {
        if (data.update.expanded?.length)
          setExpanded((previous) => previous.concat(data.update.expanded!));
        setResult(data.update);
        setElapsed(performance.now() - started.current);
        if (data.update.status !== 'running') setPaused(false);
      }
    };
    instance.onerror = () => {
      setError('Le Worker est indisponible. Rechargez la page pour réinitialiser le moteur.');
      setReady(false);
      setResult(null);
      setPaused(false);
      setGenerating(false);
      setComparing(false);
      pendingGeneration.current?.reject(new Error('Le Worker est indisponible.'));
      pendingGeneration.current = null;
    };
    return () => {
      instance.terminate();
      pendingGeneration.current?.reject(new Error('Atelier fermé.'));
      pendingGeneration.current = null;
      worker.current = null;
    };
  }, []);

  useEffect(() => {
    if (!active) return;
    const timer = setInterval(() => setElapsed(performance.now() - started.current), 100);
    return () => clearInterval(timer);
  }, [active]);

  const send = useCallback((type: 'pause' | 'resume' | 'step' | 'cancel') => {
    worker.current?.postMessage({
      type,
      runId: runId.current,
    } satisfies WorkerRequest);
  }, []);
  const reset = useCallback(() => {
    worker.current?.postMessage({
      type: 'cancel',
      runId: runId.current,
    } satisfies WorkerRequest);
    ++runId.current;
    setResult(null);
    setExpanded([]);
    setPaused(false);
    setElapsed(0);
    setGenerating(false);
    setComparing(false);
    setComparison(null);
    setComparisonProgress(null);
    setComparisonError('');
    pendingGeneration.current?.reject(new Error('Génération annulée.'));
    pendingGeneration.current = null;
  }, []);
  const start = useCallback(
    (board: Board, settings: SearchSettings, batchSize: number) => {
      if (!worker.current || !ready) return;
      ++runId.current;
      started.current = performance.now();
      setResult({ status: 'running', message: '' });
      setExpanded([]);
      setElapsed(0);
      setPaused(false);
      worker.current.postMessage({
        type: 'start',
        runId: runId.current,
        config: { ...board, ...settings },
        batchSize,
      } satisfies WorkerRequest);
    },
    [ready],
  );
  const speed = useCallback((batchSize: number) => {
    worker.current?.postMessage({
      type: 'speed',
      runId: runId.current,
      batchSize,
    } satisfies WorkerRequest);
  }, []);

  const generate = useCallback(
    (config: GenerationSettings): Promise<Board> => {
      if (!worker.current || !ready) return Promise.reject(new Error('Le moteur n’est pas prêt.'));
      pendingGeneration.current?.reject(new Error('Génération remplacée.'));
      ++runId.current;
      setGenerating(true);
      setResult(null);
      setExpanded([]);
      setPaused(false);
      setComparison(null);
      setComparisonProgress(null);
      setComparisonError('');
      return new Promise((resolve, reject) => {
        pendingGeneration.current = { resolve, reject };
        worker.current!.postMessage({
          type: 'generate',
          runId: runId.current,
          config,
        } satisfies WorkerRequest);
      });
    },
    [ready],
  );
  const compare = useCallback(
    (board: Board, settings: SearchSettings, repetitions: number) => {
      if (!worker.current || !ready) return;
      ++runId.current;
      setComparing(true);
      setResult(null);
      setExpanded([]);
      setPaused(false);
      setElapsed(0);
      setComparison(makeReport(board, settings, version, repetitions));
      setComparisonProgress(null);
      setComparisonError('');
      worker.current.postMessage({
        type: 'compare',
        runId: runId.current,
        config: { ...board, ...settings },
        repetitions,
      } satisfies WorkerRequest);
    },
    [ready, version],
  );

  return {
    ready,
    version,
    error,
    result,
    expanded,
    paused,
    elapsed,
    active,
    start,
    reset,
    send,
    speed,
    generating,
    generate,
    comparing,
    compare,
    comparison,
    comparisonProgress,
    comparisonError,
  };
}
