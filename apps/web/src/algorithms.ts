import type { Algorithm } from './protocol';

export const algorithms: {
  id: Algorithm;
  label: string;
  note: string;
  optimal: boolean;
}[] = [
  {
    id: 'astar',
    label: 'A*',
    note: 'Guidé par une heuristique admissible. Optimal sur les terrains pondérés.',
    optimal: true,
  },
  {
    id: 'dijkstra',
    label: 'Dijkstra',
    note: 'Explore par coût croissant. Optimal sur les terrains pondérés.',
    optimal: true,
  },
  {
    id: 'bfs',
    label: 'BFS',
    note: 'Explore par niveaux. Nécessite des coûts de déplacement uniformes.',
    optimal: true,
  },
  {
    id: 'dfs',
    label: 'DFS',
    note: 'Explore en profondeur. Trouve un chemin sans garantir le coût minimal.',
    optimal: false,
  },
  {
    id: 'greedy',
    label: 'Greedy Best-First',
    note: 'Privilégie la proximité estimée du but. Ne garantit pas le coût minimal.',
    optimal: false,
  },
];
