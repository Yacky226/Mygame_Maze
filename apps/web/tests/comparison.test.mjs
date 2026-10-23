import { test } from 'node:test';
import assert from 'node:assert/strict';
import { costGap, fingerprint, makeReport, summarize, comparisonCsv } from '../src/comparison.ts';
import { validateGeneration } from '../src/generation.ts';
import { blankBoard, defaultSettings, editBoard, parseProject } from '../src/model.ts';

const sample = (time) => ({
  status: 'success',
  message: '',
  cost: '6000',
  pathEdges: 6,
  optimalCostGuaranteed: true,
  metrics: { computeActiveNs: time, expandedTotal: 12, frontierPeak: 5 },
});

test('Comparison preserves raw sample order and computes exact integer statistics', () => {
  const result = summarize(
    'astar',
    ['9007199254740995', '9007199254740993', '9007199254740994'].map(sample),
  );
  assert.deepEqual(result.samplesNs, ['9007199254740995', '9007199254740993', '9007199254740994']);
  assert.equal(result.medianNs, '9007199254740994');
  assert.equal(result.minNs, '9007199254740993');
  assert.equal(result.maxNs, '9007199254740995');
  assert.equal(summarize('astar', ['3', '8', '4', '9'].map(sample)).medianNs, '6');
  assert.throws(() => summarize('astar', []));
  assert.throws(() => summarize('astar', [sample('1'), { ...sample('2'), cost: '7000' }]));
});

test('Incompatible searches have no measured sample and cost gaps handle missing or zero references', () => {
  const incompatible = summarize('bfs', [
    {
      status: 'unsupported_configuration',
      message: '',
      metrics: sample('0').metrics,
    },
  ]);
  assert.deepEqual(incompatible.samplesNs, []);
  assert.equal(incompatible.medianNs, null);
  assert.equal(costGap('28000', '6000').toFixed(2), '366.67');
  assert.equal(costGap('0', '0'), 0);
  assert.equal(costGap(null, '6000'), null);
  assert.equal(costGap('1', '0'), null);
});

test('A report owns its snapshot and fingerprint changes when search inputs change', () => {
  const board = blankBoard(7, 5);
  const report = makeReport(board, defaultSettings, '0.2.0', 3);
  assert.equal(report.fingerprint, fingerprint(board, defaultSettings));
  assert.equal(report.warmups, 1);
  assert.equal(report.complete, false);
  assert.equal(
    fingerprint(board, defaultSettings),
    fingerprint(board, { ...defaultSettings, algorithm: 'greedy' }),
  );
  assert.notEqual(report.fingerprint, fingerprint(board, { ...defaultSettings, connectivity: 8 }));
  assert.notEqual(report.fingerprint, fingerprint(board, { ...defaultSettings, maxExpansions: 1 }));
  board.blocked[0] = 1;
  assert.equal(report.board.blocked[0], 0);
  assert.notEqual(report.fingerprint, fingerprint(board, defaultSettings));
});

test('CSV names its units, provenance and sample counts, and JSON retains the full snapshot', () => {
  const board = {
    ...blankBoard(7, 5),
    generation: {
      algorithm: 'prim',
      seed: 4294967295,
      version: 1,
      modified: false,
    },
  };
  const report = {
    ...makeReport(board, defaultSettings, '0.2.0', 3),
    complete: true,
    rows: [summarize('astar', ['5', '3', '4'].map(sample))],
  };
  const csv = comparisonCsv(report);
  assert.ok(csv.startsWith('\ufeff"schema_version"'));
  assert.ok(csv.includes('"cost_milli_steps"'));
  assert.ok(csv.includes('"median_compute_active_ns"'));
  assert.ok(csv.includes('"prim","4294967295","1","false"'));
  assert.ok(csv.includes('"3","1","4","3","5"'));
  assert.deepEqual(JSON.parse(JSON.stringify(report)), report);
});

test('Generation rejects even dimensions and out-of-range seeds at the web boundary', () => {
  const config = { algorithm: 'prim', width: 41, height: 25, seed: 4294967295 };
  assert.doesNotThrow(() => validateGeneration(config));
  for (const invalid of [
    { width: 40 },
    { height: 257 },
    { width: 1 },
    { seed: -1 },
    { seed: 4294967296 },
    { seed: NaN },
    { seed: 1.5 },
    { algorithm: 'unknown' },
  ])
    assert.throws(() => validateGeneration({ ...config, ...invalid }));
});

test('Generated provenance survives project files and is marked modified only after actual edits', () => {
  const board = {
    ...blankBoard(7, 5),
    generation: { algorithm: 'kruskal', seed: 0, version: 1, modified: false },
  };
  const project = {
    schemaVersion: 1,
    title: 'Génération',
    board,
    settings: { ...defaultSettings, algorithm: 'greedy' },
    appearance: { view: 'technical' },
  };
  assert.deepEqual(parseProject(JSON.stringify(project)), project);
  assert.throws(() =>
    parseProject(JSON.stringify({ ...project, appearance: { view: 'unknown' } })),
  );
  assert.equal(editBoard(board, [board.start], 'wall', 5).generation.modified, false);
  assert.equal(editBoard(board, [0], 'wall', 5).generation.modified, true);
  assert.equal(board.generation.modified, false);
  for (const invalid of [
    { seed: -1 },
    { seed: 4294967296 },
    { algorithm: 'other' },
    { version: 2 },
    { modified: 'false' },
  ])
    assert.throws(() =>
      parseProject(
        JSON.stringify({
          ...project,
          board: { ...board, generation: { ...board.generation, ...invalid } },
        }),
      ),
    );
});
