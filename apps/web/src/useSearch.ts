import { useCallback, useEffect, useRef, useState } from 'react';
import type {
  Board,
  EngineUpdate,
  SearchSettings,
  WorkerRequest,
  WorkerResponse,
} from './protocol';

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
        return;
      }
      if (data.runId !== runId.current) return;
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
    };
    return () => {
      instance.terminate();
      worker.current = null;
    };
  }, []);

  useEffect(() => {
    if (!active) return;
    const timer = setInterval(() => setElapsed(performance.now() - started.current), 100);
    return () => clearInterval(timer);
  }, [active]);

  const send = useCallback((type: 'pause' | 'resume' | 'step' | 'cancel') => {
    worker.current?.postMessage({ type, runId: runId.current } satisfies WorkerRequest);
  }, []);
  const reset = useCallback(() => {
    worker.current?.postMessage({ type: 'cancel', runId: runId.current } satisfies WorkerRequest);
    ++runId.current;
    setResult(null);
    setExpanded([]);
    setPaused(false);
    setElapsed(0);
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
  };
}
