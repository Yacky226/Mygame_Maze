export type Algorithm = 'astar' | 'dijkstra' | 'bfs' | 'dfs';
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
  cost?: string | null;
  optimalCostGuaranteed?: boolean;
  metrics?: Metrics;
}
export type WorkerRequest =
  | { type: 'start'; runId: number; config: Board & SearchSettings; batchSize: number }
  | { type: 'pause' | 'resume' | 'step' | 'cancel'; runId: number }
  | { type: 'speed'; runId: number; batchSize: number };
export type WorkerResponse =
  | { type: 'ready'; engineVersion: string }
  | { type: 'fatal'; message: string }
  | { type: 'paused' | 'resumed'; runId: number }
  | { type: 'update'; runId: number; update: EngineUpdate };

export interface WasmEngine {
  start(config: Board & SearchSettings): EngineUpdate;
  step(budget: number): EngineUpdate;
  cancel(): EngineUpdate;
  delete(): void;
}
export interface WasmModule {
  Engine: new () => WasmEngine;
  engineVersion(): string;
}
