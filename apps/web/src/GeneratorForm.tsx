import { useState } from 'react';
import { ArrowRight, Shuffle } from 'lucide-react';
import { generators, validateGeneration } from './generation';
import type { GenerationSettings } from './protocol';

export function GeneratorForm({
  initial,
  busy,
  onGenerate,
}: {
  initial: GenerationSettings;
  busy: boolean;
  onGenerate(config: GenerationSettings): Promise<void>;
}) {
  const [config, setConfig] = useState(initial);
  const [seed, setSeed] = useState(String(initial.seed));
  const [error, setError] = useState('');
  return (
    <form
      className="generator-form"
      onSubmit={async (event) => {
        event.preventDefault();
        setError('');
        try {
          if (!/^\d+$/.test(seed))
            throw new Error('La graine doit contenir uniquement des chiffres.');
          const request = { ...config, seed: Number(seed) };
          validateGeneration(request);
          await onGenerate(request);
        } catch (failure) {
          setError(failure instanceof Error ? failure.message : 'La génération a échoué.');
        }
      }}
    >
      <p className="modal-intro">
        Trois façons de construire un labyrinthe. Conservez la graine pour retrouver exactement la
        même carte.
      </p>
      <div className="generator-choices">
        {generators.map((generator) => (
          <label key={generator.id} className={config.algorithm === generator.id ? 'selected' : ''}>
            <input
              type="radio"
              name="generator"
              value={generator.id}
              checked={config.algorithm === generator.id}
              disabled={busy}
              onChange={() =>
                setConfig((previous) => ({
                  ...previous,
                  algorithm: generator.id,
                }))
              }
            />
            <span>
              <strong>{generator.label}</strong>
              <small>{generator.note}</small>
            </span>
          </label>
        ))}
      </div>
      <div className="generator-dimensions">
        <label>
          Colonnes du labyrinthe
          <input
            type="number"
            min="3"
            max="255"
            step="2"
            value={config.width}
            disabled={busy}
            onChange={(event) =>
              setConfig((previous) => ({
                ...previous,
                width: Number(event.target.value),
              }))
            }
          />
        </label>
        <label>
          Lignes du labyrinthe
          <input
            type="number"
            min="3"
            max="255"
            step="2"
            value={config.height}
            disabled={busy}
            onChange={(event) =>
              setConfig((previous) => ({
                ...previous,
                height: Number(event.target.value),
              }))
            }
          />
        </label>
      </div>
      <div className="seed-control">
        <label>
          Graine de génération
          <input
            inputMode="numeric"
            pattern="[0-9]+"
            value={seed}
            disabled={busy}
            onChange={(event) => setSeed(event.target.value)}
          />
        </label>
        <button
          type="button"
          className="outline-button"
          disabled={busy}
          onClick={() => setSeed(String(crypto.getRandomValues(new Uint32Array(1))[0]))}
          aria-label="Choisir une nouvelle graine"
        >
          <Shuffle size={16} />
        </button>
      </div>
      <p className="field-note">
        Dimensions impaires, murs fermés en bordure, poids uniformes. Un seul passage relie chaque
        paire de cases libres en 4 directions. Le départ est placé en (1, 1), l’arrivée à une case
        la plus éloignée.
      </p>
      {error && (
        <p className="notice warning" role="alert">
          {error}
        </p>
      )}
      <button className="run-button generator-submit" type="submit" disabled={busy}>
        {busy ? 'Génération en cours…' : 'Générer la carte'}
        <ArrowRight size={16} />
      </button>
    </form>
  );
}
