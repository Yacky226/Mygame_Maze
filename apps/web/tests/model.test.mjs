import { test } from 'node:test';
import assert from 'node:assert/strict';
import {
  bfsCompatible,
  blankBoard,
  defaultSettings,
  editBoard,
  lineNodes,
  parseProject,
  presetBoard,
  randomBoard,
} from '../src/model.ts';

test('Editing protects endpoints and does not mutate the previous board', () => {
  const board = blankBoard(10, 8);
  const edited = editBoard(board, [0, board.start, board.goal], 'wall', 5);
  assert.equal(edited.blocked[0], 1);
  assert.equal(edited.blocked[board.start], 0);
  assert.equal(edited.blocked[board.goal], 0);
  assert.equal(board.blocked[0], 0);
  const start = editBoard(edited, [0], 'start', 5);
  assert.equal(start.start, 0);
  assert.equal(start.blocked[0], 0);
  const weighted = editBoard(start, [1], 'terrain', 9);
  assert.equal(weighted.weights[1], 9);
  assert.equal(editBoard(weighted, [1], 'erase', 9).weights[1], 1);
});

test('Pointer interpolation fills fast strokes in either direction', () => {
  assert.deepEqual(lineNodes(0, 4, 5), [0, 1, 2, 3, 4]);
  assert.deepEqual(lineNodes(24, 0, 5), [24, 18, 12, 6, 0]);
  assert.deepEqual(lineNodes(0, 0, 5), [0]);
});

test('Import round-trips projects and refuses malformed or excessive input', () => {
  const project = {
    schemaVersion: 1,
    title: 'Une expérience',
    board: presetBoard('weighted'),
    settings: defaultSettings,
  };
  assert.deepEqual(parseProject(JSON.stringify(project)), project);
  for (const invalid of [
    null,
    {},
    { ...project, schemaVersion: 2 },
    { ...project, board: { ...project.board, width: 257 } },
    { ...project, board: { ...project.board, start: -1 } },
    { ...project, board: { ...project.board, weights: [0] } },
    { ...project, settings: { ...defaultSettings, connectivity: 6 } },
    { ...project, settings: { ...defaultSettings, maxExpansions: -1 } },
  ]) {
    assert.throws(() => parseProject(JSON.stringify(invalid)));
  }
  assert.throws(() => parseProject(' '.repeat(2 * 1024 * 1024 + 1)));
});

test('BFS eligibility matches the transition model', () => {
  assert.equal(bfsCompatible(blankBoard(), defaultSettings), true);
  assert.equal(bfsCompatible(presetBoard('weighted'), defaultSettings), false);
  assert.equal(bfsCompatible(blankBoard(), { ...defaultSettings, connectivity: 8 }), false);
});

test('Random boards are reproducible and keep endpoints free', () => {
  const board = blankBoard();
  assert.deepEqual(randomBoard(board, 123), randomBoard(board, 123));
  assert.equal(randomBoard(board, 123).blocked[board.start], 0);
  assert.equal(randomBoard(board, 123).blocked[board.goal], 0);
});
