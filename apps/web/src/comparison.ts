import type {
  Board,
  ComparisonReport,
  ComparisonRow,
  EngineUpdate,
  SearchSettings,
} from './protocol';

export function summarize(
  algorithm: ComparisonRow['algorithm'],
  samples: EngineUpdate[],
): ComparisonRow {
  if (!samples.length) throw new Error('Aucun échantillon de comparaison.');
  const result = samples[0];
  if (
    samples.some(
      (sample) =>
        sample.status !== result.status ||
        sample.cost !== result.cost ||
        sample.metrics?.expandedTotal !== result.metrics?.expandedTotal ||
        sample.pathEdges !== result.pathEdges,
    )
  )
    throw new Error('Les répétitions ne donnent pas le même résultat.');
  const compatible =
    result.status !== 'unsupported_configuration' && result.status !== 'invalid_request';
  const rawTimes = compatible
    ? samples.map((sample) => BigInt(sample.metrics?.computeActiveNs ?? '0'))
    : [];
  const times = [...rawTimes].sort((a, b) => (a < b ? -1 : a > b ? 1 : 0));
  const median = times.length
    ? times.length % 2
      ? times[Math.floor(times.length / 2)]
      : (times[times.length / 2 - 1] + times[times.length / 2]) / 2n
    : null;
  return {
    algorithm,
    status: result.status,
    cost: result.cost ?? null,
    optimalCostGuaranteed: result.optimalCostGuaranteed ?? false,
    pathEdges:
      result.status === 'success'
        ? (result.pathEdges ?? Math.max(0, (result.path?.length ?? 1) - 1))
        : null,
    metrics: result.metrics,
    samplesNs: rawTimes.map(String),
    medianNs: median?.toString() ?? null,
    minNs: times.length ? String(times[0]) : null,
    maxNs: times.length ? String(times[times.length - 1]) : null,
  };
}

export function fingerprint(board: Board, settings: Omit<SearchSettings, 'algorithm'>): string {
  let hash = 2166136261;
  const add = (value: number) => {
    for (let byte = 0; byte < 4; byte++)
      hash = Math.imul(hash ^ ((value >>> (byte * 8)) & 255), 16777619) >>> 0;
  };
  [
    board.width,
    board.height,
    board.start,
    board.goal,
    settings.connectivity,
    Number(settings.allowCornerCutting),
    settings.maxExpansions,
  ].forEach(add);
  board.blocked.forEach(add);
  board.weights.forEach(add);
  return hash.toString(16).padStart(8, '0');
}

export function makeReport(
  board: Board,
  settings: SearchSettings,
  engineVersion: string,
  repetitions: number,
): ComparisonReport {
  const movement = {
    connectivity: settings.connectivity,
    allowCornerCutting: settings.allowCornerCutting,
    maxExpansions: settings.maxExpansions,
  };
  return {
    schemaVersion: 1,
    engineVersion,
    createdAt: new Date().toISOString(),
    board: structuredClone(board),
    settings: movement,
    repetitions,
    warmups: 1,
    complete: false,
    rows: [],
    fingerprint: fingerprint(board, movement),
  };
}

export function costGap(cost: string | null, reference: string | null): number | null {
  if (cost === null || reference === null) return null;
  if (reference === '0') return cost === '0' ? 0 : null;
  return (Number(cost) / Number(reference) - 1) * 100;
}

export function comparisonCsv(report: ComparisonReport): string {
  const header = [
    'schema_version',
    'engine_version',
    'created_at',
    'fingerprint',
    'complete',
    'width',
    'height',
    'generator',
    'seed',
    'generator_version',
    'map_modified',
    'connectivity',
    'corner_cutting',
    'max_expansions',
    'algorithm',
    'status',
    'cost_milli_steps',
    'optimal_cost_guaranteed',
    'path_edges',
    'expanded_total',
    'discovered_unique',
    'frontier_peak',
    'samples',
    'warmups',
    'median_compute_active_ns',
    'min_compute_active_ns',
    'max_compute_active_ns',
  ];
  const quote = (value: unknown) => `"${String(value ?? '').replaceAll('"', '""')}"`;
  const rows = report.rows.map((row) => [
    1,
    report.engineVersion,
    report.createdAt,
    report.fingerprint,
    report.complete,
    report.board.width,
    report.board.height,
    report.board.generation?.algorithm,
    report.board.generation?.seed,
    report.board.generation?.version,
    report.board.generation?.modified,
    report.settings.connectivity,
    report.settings.allowCornerCutting,
    report.settings.maxExpansions,
    row.algorithm,
    row.status,
    row.cost,
    row.optimalCostGuaranteed,
    row.pathEdges,
    row.metrics?.expandedTotal,
    row.metrics?.discoveredUnique,
    row.metrics?.frontierPeak,
    row.samplesNs.length,
    report.warmups,
    row.medianNs,
    row.minNs,
    row.maxNs,
  ]);
  return '\ufeff' + [header, ...rows].map((row) => row.map(quote).join(',')).join('\r\n') + '\r\n';
}
