export type Algorithm = 'astar' | 'dijkstra' | 'bfs' | 'dfs' | 'greedy';
export type Generator = 'backtracker' | 'prim' | 'kruskal';
export interface GenerationSettings {
  width: number;
  height: number;
  algorithm: Generator;
  seed: number;
}
export interface GenerationMetadata {
  algorithm: Generator;
  seed: number;
  version: 1;
  modified: boolean;
}
export type EngineStatus =
  | 'running'
  | 'success'
  | 'no_path'
  | 'cancelled'
  | 'resource_limit'
  | 'invalid_request'
  | 'unsupported_configuration'
  | 'internal_error';

export interface Board {
  width: number;
  height: number;
  blocked: number[];
  weights: number[];
  start: number;
  goal: number;
  generation?: GenerationMetadata;
}
export interface SearchSettings {
  algorithm: Algorithm;
  connectivity: 4 | 8;
  allowCornerCutting: boolean;
  maxExpansions: number;
}
export interface Metrics {
  discoveredUnique: number;
  expandedTotal: number;
  relaxations: number;
  reopened: number;
  stalePops: number;
  frontierPeak: number;
  computeActiveNs: string;
}
export interface EngineUpdate {
  status: EngineStatus;
  message: string;
  expanded?: number[];
  path?: number[];
  pathEdges?: number;
  cost?: string | null;
  optimalCostGuaranteed?: boolean;
  metrics?: Metrics;
}
export type EngineConfig = Board &
  SearchSettings & { recordExpansions?: boolean; includePath?: boolean };
export interface ComparisonRow {
  algorithm: Algorithm;
  status: EngineStatus;
  cost: string | null;
  optimalCostGuaranteed: boolean;
  pathEdges: number | null;
  metrics?: Metrics;
  samplesNs: string[];
  medianNs: string | null;
  minNs: string | null;
  maxNs: string | null;
}
export interface ComparisonProgress {
  completed: number;
  total: number;
  algorithm: Algorithm;
}
export interface ComparisonReport {
  schemaVersion: 1;
  engineVersion: string;
  createdAt: string;
  board: Board;
  settings: Omit<SearchSettings, 'algorithm'>;
  repetitions: number;
  warmups: 1;
  complete: boolean;
  rows: ComparisonRow[];
  fingerprint: string;
}
export type WorkerRequest =
  | { type: 'generate'; runId: number; config: GenerationSettings }
  | {
      type: 'compare';
      runId: number;
      config: Board & SearchSettings;
      repetitions: number;
    }
  | {
      type: 'start';
      runId: number;
      config: Board & SearchSettings;
      batchSize: number;
    }
  | { type: 'pause' | 'resume' | 'step' | 'cancel'; runId: number }
  | { type: 'speed'; runId: number; batchSize: number };
export type WorkerResponse =
  | { type: 'generated'; runId: number; board: Board }
  | { type: 'operation-error'; runId: number; message: string }
  | {
      type: 'comparison-progress';
      runId: number;
      progress: ComparisonProgress;
      rows: ComparisonRow[];
    }
  | {
      type: 'comparison-done';
      runId: number;
      rows: ComparisonRow[];
      complete: boolean;
    }
  | { type: 'ready'; engineVersion: string }
  | { type: 'fatal'; message: string }
  | { type: 'paused' | 'resumed'; runId: number }
  | { type: 'update'; runId: number; update: EngineUpdate };

export interface WasmEngine {
  start(config: EngineConfig): EngineUpdate;
  step(budget: number): EngineUpdate;
  cancel(): EngineUpdate;
  delete(): void;
}
export interface WasmModule {
  Engine: new () => WasmEngine;
  engineVersion(): string;
  generateMaze(config: GenerationSettings): {
    status: string;
    board?: Board;
    message?: string;
  };
}
