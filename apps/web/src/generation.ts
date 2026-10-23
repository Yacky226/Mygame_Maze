import type { GenerationSettings, Generator } from './protocol';

export const generators: { id: Generator; label: string; note: string }[] = [
  {
    id: 'backtracker',
    label: 'Backtracking',
    note: 'Des couloirs longs et sinueux, creusés en profondeur.',
  },
  {
    id: 'prim',
    label: 'Prim aléatoire',
    note: 'Une croissance depuis la frontière, avec de nombreuses ramifications.',
  },
  {
    id: 'kruskal',
    label: 'Kruskal aléatoire',
    note: 'Des passages dispersés qui réunissent progressivement les régions.',
  },
];
export function validateGeneration(config: GenerationSettings): void {
  if (
    ![config.width, config.height].every(
      (value) => Number.isInteger(value) && value >= 3 && value <= 255 && value % 2 === 1,
    )
  )
    throw new Error('Choisissez des dimensions impaires entre 3 et 255.');
  if (!Number.isInteger(config.seed) || config.seed < 0 || config.seed > 4294967295)
    throw new Error('La graine doit être un entier de 0 à 4 294 967 295.');
  if (!generators.some((generator) => generator.id === config.algorithm))
    throw new Error('Générateur inconnu.');
}
