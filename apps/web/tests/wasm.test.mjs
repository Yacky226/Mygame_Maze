import { test } from 'node:test';
import assert from 'node:assert/strict';
import { readFileSync, writeFileSync, mkdtempSync, unlinkSync, rmdirSync } from 'node:fs';
import { tmpdir } from 'node:os';
import path from 'node:path';
import { fileURLToPath } from 'node:url';
import { spawnSync } from 'node:child_process';
import { defaultSettings, presetBoard } from '../src/model.ts';
import createModule from '../public/engine/maze_engine.mjs';

const module = await createModule({
  wasmBinary: readFileSync(new URL('../public/engine/maze_engine.wasm', import.meta.url)),
});
const engine = new module.Engine();
const cli =
  process.env.MAZE_CLI ||
  fileURLToPath(new URL('../../../build/core-debug/apps/cli/Debug/maze_cli.exe', import.meta.url));
function run(board, settings = defaultSettings, budget = 7) {
  const expanded = [];
  let result = engine.start({ ...board, ...settings });
  for (let count = 0; result.status === 'running'; count++) {
    assert.ok(count < 100000, 'bounded completion');
    result = engine.step(budget);
    expanded.push(...result.expanded);
  }
  return { ...result, expanded };
}

test('WASM weighted detour, diagonals, no-path and unsupported BFS', () => {
  assert.equal(run(presetBoard('weighted')).cost, '6000');
  assert.equal(
    run(presetBoard('weighted'), { ...defaultSettings, algorithm: 'bfs' }).status,
    'unsupported_configuration',
  );
  const settings = { ...defaultSettings, connectivity: 8 };
  assert.equal(run(presetBoard('corners'), settings).status, 'no_path');
  assert.equal(run(presetBoard('corners'), { ...settings, allowCornerCutting: true }).cost, '1414');
  assert.equal(run(presetBoard('no-path')).status, 'no_path');
});

test('WASM stepping is deterministic, incremental and cancellable', () => {
  const board = presetBoard('passages');
  const slow = run(board, defaultSettings, 1);
  const fast = run(board, defaultSettings, 4096);
  assert.deepEqual(slow.path, fast.path);
  assert.deepEqual(slow.expanded, fast.expanded);
  assert.equal(slow.metrics.expandedTotal, slow.expanded.length);
  engine.start({ ...board, ...defaultSettings });
  assert.equal(engine.step(1).expanded.length, 1);
  assert.equal(engine.cancel().status, 'cancelled');
  assert.equal(engine.step(1).status, 'cancelled');
  assert.equal(engine.cancel().expanded.length, 0);
  assert.equal(run(presetBoard('weighted')).cost, '6000');
});

test('WASM validates the boundary and reports resource limits distinctly', () => {
  assert.equal(
    engine.start({ ...presetBoard('weighted'), ...defaultSettings, width: 300 }).status,
    'invalid_request',
  );
  assert.equal(
    engine.start({
      ...presetBoard('weighted'),
      ...defaultSettings,
      weights: [NaN],
    }).status,
    'invalid_request',
  );
  assert.equal(
    engine.start({
      ...presetBoard('weighted'),
      ...defaultSettings,
      connectivity: 6,
    }).status,
    'invalid_request',
  );
  assert.equal(
    run(presetBoard('passages'), { ...defaultSettings, maxExpansions: 1 }).status,
    'resource_limit',
  );
});

test('Native CLI and WASM agree on deterministic weighted maps', () => {
  const temp = mkdtempSync(path.join(tmpdir(), 'maze-parity-'));
  const filename = path.join(temp, 'map.txt');
  let seed = 0x51a7e123;
  const random = () => (seed = (Math.imul(seed, 1664525) + 1013904223) >>> 0);
  try {
    for (let sample = 0; sample < 24; sample++) {
      const width = 3 + (random() % 5),
        height = 3 + (random() % 5);
      const board = {
        width,
        height,
        start: 0,
        goal: width * height - 1,
        blocked: [],
        weights: [],
      };
      const rows = [];
      for (let y = 0; y < height; y++) {
        let row = '';
        for (let x = 0; x < width; x++) {
          const node = y * width + x,
            weight = 1 + (random() % 9);
          const blocked = node !== board.start && node !== board.goal && random() % 5 === 0;
          board.blocked.push(blocked ? 1 : 0);
          board.weights.push(node === board.start || node === board.goal ? 1 : weight);
          row +=
            node === board.start ? 'S' : node === board.goal ? 'G' : blocked ? '#' : String(weight);
        }
        rows.push(row);
      }
      writeFileSync(filename, rows.join('\n'));
      for (const algorithm of ['astar', 'dijkstra', 'greedy'])
        for (const connectivity of [4, 8]) {
          const settings = {
            ...defaultSettings,
            algorithm,
            connectivity,
            allowCornerCutting: sample % 2 === 0,
          };
          const args = [filename, '--algorithm', algorithm];
          if (connectivity === 8) {
            args.push('--diagonal');
            if (settings.allowCornerCutting) args.push('--allow-corner-cutting');
          }
          const native = spawnSync(cli, args, { encoding: 'utf8' });
          assert.ifError(native.error);
          const expected = JSON.parse(native.stdout);
          const actual = run(board, settings);
          assert.equal(
            actual.status,
            expected.status,
            `sample ${sample}, ${algorithm}, ${connectivity}`,
          );
          assert.equal(actual.cost, expected.cost);
          assert.deepEqual(
            actual.path,
            expected.path.map(([x, y]) => y * width + x),
          );
          assert.equal(actual.metrics.expandedTotal, expected.metrics.expandedTotal);
        }
    }
  } finally {
    try {
      unlinkSync(filename);
    } finally {
      rmdirSync(temp);
    }
  }
});

test('Greedy returns a valid but more costly route and benchmark mode suppresses display payloads', () => {
  const board = presetBoard('weighted');
  const greedy = run(board, { ...defaultSettings, algorithm: 'greedy' });
  assert.equal(greedy.cost, '28000');
  assert.equal(greedy.optimalCostGuaranteed, false);
  const reference = run(board);
  let quiet = engine.start({
    ...board,
    ...defaultSettings,
    recordExpansions: false,
    includePath: false,
  });
  while (quiet.status === 'running') {
    quiet = engine.step(4096);
    assert.deepEqual(quiet.expanded, []);
  }
  assert.deepEqual(quiet.path, []);
  assert.equal(quiet.pathEdges, reference.path.length - 1);
  assert.equal(quiet.cost, reference.cost);
  assert.equal(quiet.metrics.expandedTotal, reference.metrics.expandedTotal);
});

test('Native and WASM generators produce the exact same version-one maps and endpoints', () => {
  const generatorCli =
    process.env.MAZE_GENERATE ||
    path.join(
      path.dirname(cli),
      process.platform === 'win32' ? 'maze_generate.exe' : 'maze_generate',
    );
  for (const algorithm of ['backtracker', 'prim', 'kruskal'])
    for (const seed of [0, 42, 4294967295]) {
      const config = { algorithm, seed, width: 21, height: 15 };
      const generated = module.generateMaze(config);
      assert.equal(generated.status, 'success');
      const board = generated.board;
      const native = spawnSync(
        generatorCli,
        ['--generator', algorithm, '--seed', String(seed), '--width', '21', '--height', '15'],
        { encoding: 'utf8' },
      );
      assert.ifError(native.error);
      assert.equal(native.status, 0, native.stderr);
      const nativeRows = native.stdout.replaceAll('\r', '').trimEnd().split('\n');
      const actualRows = Array.from({ length: board.height }, (_, y) =>
        Array.from({ length: board.width }, (_, x) => {
          const node = y * board.width + x;
          return node === board.start && node === board.goal
            ? 'B'
            : node === board.start
              ? 'S'
              : node === board.goal
                ? 'G'
                : board.blocked[node]
                  ? '#'
                  : '.';
        }).join(''),
      );
      assert.deepEqual(actualRows, nativeRows, `${algorithm}, seed ${seed}`);
      assert.deepEqual(board.generation, {
        algorithm,
        seed,
        version: 1,
        modified: false,
      });
      assert.ok(board.weights.every((weight) => weight === 1));
      assert.equal(run(board).status, 'success');
    }
  assert.equal(
    module.generateMaze({ algorithm: 'prim', seed: 0, width: 3, height: 3 }).board.start,
    module.generateMaze({ algorithm: 'prim', seed: 0, width: 3, height: 3 }).board.goal,
  );
  for (const patch of [
    { width: 4 },
    { height: 257 },
    { seed: -1 },
    { seed: 4294967296 },
    { algorithm: 'unknown' },
  ])
    assert.equal(
      module.generateMaze({
        algorithm: 'prim',
        width: 41,
        height: 25,
        seed: 42,
        ...patch,
      }).status,
      'invalid_request',
    );
});
