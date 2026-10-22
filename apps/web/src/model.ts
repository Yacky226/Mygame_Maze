import type { Algorithm, Board, SearchSettings } from './protocol';

export type Tool = 'wall' | 'erase' | 'start' | 'goal' | 'terrain';
export const defaultSettings: SearchSettings = {
  algorithm: 'astar',
  connectivity: 4,
  allowCornerCutting: false,
  maxExpansions: 0,
};

export function blankBoard(width = 40, height = 24): Board {
  if (
    !Number.isInteger(width) ||
    !Number.isInteger(height) ||
    width < 2 ||
    height < 2 ||
    width > 256 ||
    height > 256
  )
    throw new Error('La carte doit mesurer de 2 à 256 cases par côté.');
  const row = Math.floor(height / 2);
  return {
    width,
    height,
    blocked: Array(width * height).fill(0),
    weights: Array(width * height).fill(1),
    start: row * width + Math.max(0, Math.floor(width * 0.15)),
    goal: row * width + Math.min(width - 1, Math.floor(width * 0.83)),
  };
}

export function editBoard(board: Board, nodes: number[], tool: Tool, weight: number): Board {
  const next = { ...board, blocked: [...board.blocked], weights: [...board.weights] };
  for (const node of nodes) {
    if (!Number.isInteger(node) || node < 0 || node >= next.blocked.length) continue;
    if (tool === 'start') {
      next.start = node;
      next.blocked[node] = 0;
    } else if (tool === 'goal') {
      next.goal = node;
      next.blocked[node] = 0;
    } else if (node !== next.start && node !== next.goal) {
      if (tool === 'wall') next.blocked[node] = 1;
      if (tool === 'erase') {
        next.blocked[node] = 0;
        next.weights[node] = 1;
      }
      if (tool === 'terrain') {
        next.blocked[node] = 0;
        next.weights[node] = weight;
      }
    }
  }
  return next;
}

// Fill gaps between pointer events, including fast drags and diagonals.
export function lineNodes(from: number, to: number, width: number): number[] {
  let x = from % width,
    y = Math.floor(from / width);
  const endX = to % width,
    endY = Math.floor(to / width);
  const dx = Math.abs(endX - x),
    dy = -Math.abs(endY - y);
  const sx = x < endX ? 1 : -1,
    sy = y < endY ? 1 : -1;
  let error = dx + dy;
  const nodes: number[] = [];
  while (true) {
    nodes.push(y * width + x);
    if (x === endX && y === endY) break;
    const doubleError = 2 * error;
    if (doubleError >= dy) {
      error += dy;
      x += sx;
    }
    if (doubleError <= dx) {
      error += dx;
      y += sy;
    }
  }
  return nodes;
}

export type Preset = 'passages' | 'weighted' | 'blank' | 'corners' | 'no-path';
export const presetNames: Record<Preset, string> = {
  passages: 'Les deux passages',
  weighted: 'Le détour pondéré',
  blank: 'Page blanche',
  corners: 'Le coin infranchissable',
  'no-path': 'Une arrivée isolée',
};
export function presetBoard(preset: Preset): Board {
  if (preset === 'blank') return blankBoard();
  if (preset === 'weighted')
    return {
      width: 5,
      height: 3,
      start: 5,
      goal: 9,
      blocked: Array(15).fill(0),
      weights: [1, 1, 1, 1, 1, 1, 9, 9, 9, 1, 1, 1, 1, 1, 1],
    };
  if (preset === 'corners')
    return { width: 2, height: 2, start: 0, goal: 3, blocked: [0, 1, 1, 0], weights: [1, 1, 1, 1] };
  const board = blankBoard();
  if (preset === 'no-path') {
    for (const node of [
      board.goal - 1,
      board.goal + 1,
      board.goal - board.width,
      board.goal + board.width,
    ])
      board.blocked[node] = 1;
    return board;
  }
  for (let y = 3; y < 21; y++) {
    if (y < 6 || y > 8) board.blocked[y * 40 + 14] = 1;
    if (y < 16 || y > 18) board.blocked[y * 40 + 26] = 1;
  }
  for (let y = 9; y < 15; y++) for (let x = 18; x < 23; x++) board.weights[y * 40 + x] = 5;
  return board;
}

export function randomBoard(board: Board, seed = Date.now()): Board {
  let state = seed >>> 0;
  const next = {
    ...board,
    blocked: board.blocked.map(() => {
      state = (Math.imul(state, 1664525) + 1013904223) >>> 0;
      return state / 4294967296 < 0.24 ? 1 : 0;
    }),
    weights: Array(board.width * board.height).fill(1),
  };
  next.blocked[next.start] = next.blocked[next.goal] = 0;
  return next;
}

export function bfsCompatible(board: Board, settings: SearchSettings): boolean {
  if (settings.connectivity === 8) return false; // diagonal costs are 1414, orthogonal costs 1000
  const weights = board.weights.filter((_, index) => !board.blocked[index]);
  return weights.every((weight) => weight === weights[0]);
}

export interface ProjectFile {
  schemaVersion: 1;
  title: string;
  board: Board;
  settings: SearchSettings;
}
const algorithms: Algorithm[] = ['astar', 'dijkstra', 'bfs', 'dfs'];
export function parseProject(text: string): ProjectFile {
  if (text.length > 2 * 1024 * 1024) throw new Error('Le fichier dépasse 2 Mio.');
  const input = JSON.parse(text) as ProjectFile;
  if (
    !input ||
    input.schemaVersion !== 1 ||
    typeof input.title !== 'string' ||
    input.title.length > 100
  )
    throw new Error('Format de projet non reconnu (version attendue : 1).');
  const board = input.board;
  if (
    !board ||
    !Number.isInteger(board.width) ||
    !Number.isInteger(board.height) ||
    board.width < 2 ||
    board.height < 2 ||
    board.width > 256 ||
    board.height > 256
  )
    throw new Error('Dimensions invalides : de 2 à 256 cases par côté.');
  const count = board.width * board.height;
  if (
    !Array.isArray(board.blocked) ||
    !Array.isArray(board.weights) ||
    board.blocked.length !== count ||
    board.weights.length !== count ||
    board.blocked.some((value) => value !== 0 && value !== 1) ||
    board.weights.some((value) => !Number.isInteger(value) || value < 1 || value > 1000)
  )
    throw new Error('Données des cases invalides.');
  if (
    ![board.start, board.goal].every(
      (node) => Number.isInteger(node) && node >= 0 && node < count && !board.blocked[node],
    )
  )
    throw new Error('Le départ et l’arrivée doivent se trouver sur des cases libres.');
  const settings = input.settings;
  if (
    !settings ||
    !algorithms.includes(settings.algorithm) ||
    ![4, 8].includes(settings.connectivity) ||
    typeof settings.allowCornerCutting !== 'boolean' ||
    !Number.isInteger(settings.maxExpansions) ||
    settings.maxExpansions < 0 ||
    settings.maxExpansions > 1000000
  )
    throw new Error('Paramètres de recherche invalides.');
  // Rebuild explicitly; imported properties cannot become application state.
  return {
    schemaVersion: 1,
    title: input.title,
    board: {
      width: board.width,
      height: board.height,
      blocked: [...board.blocked],
      weights: [...board.weights],
      start: board.start,
      goal: board.goal,
    },
    settings: {
      algorithm: settings.algorithm,
      connectivity: settings.connectivity,
      allowCornerCutting: settings.allowCornerCutting,
      maxExpansions: settings.maxExpansions,
    },
  };
}
