import { useState } from 'react';
import { ArrowDownToLine, ChartNoAxesColumnIncreasing, CircleStop, Play } from 'lucide-react';
import { algorithms } from './algorithms';
import { costGap } from './comparison';
import type { ComparisonProgress, ComparisonReport, EngineStatus } from './protocol';

const statuses: Record<EngineStatus, string> = {
  running: 'En cours',
  success: 'Chemin trouvé',
  no_path: 'Sans chemin',
  cancelled: 'Annulé',
  resource_limit: 'Budget épuisé',
  invalid_request: 'Configuration invalide',
  unsupported_configuration: 'Non compatible',
  internal_error: 'Erreur du moteur',
};
const format = (value: number) =>
  new Intl.NumberFormat('fr-FR', { maximumFractionDigits: 2 }).format(value);
function duration(ns: string | null) {
  if (ns === null) return '—';
  return Number(ns) < 1000000
    ? `${format(Number(ns) / 1000)} µs`
    : `${format(Number(ns) / 1000000)} ms`;
}

export function ComparisonPanel({
  busy,
  disabled,
  report,
  progress,
  error,
  onStart,
  onCancel,
  onExport,
}: {
  busy: boolean;
  disabled: boolean;
  report: ComparisonReport | null;
  progress: ComparisonProgress | null;
  error: string;
  onStart(repetitions: number): void;
  onCancel(): void;
  onExport(report: ComparisonReport, format: 'json' | 'csv'): void;
}) {
  const [repetitions, setRepetitions] = useState(5);
  const reference =
    report?.rows.find((row) => row.algorithm === 'dijkstra' && row.status === 'success')?.cost ??
    null;
  const maximum = Math.max(1, ...(report?.rows.map((row) => Number(row.medianNs ?? 0)) ?? []));
  return (
    <section className="comparison-panel" aria-labelledby="comparison-heading">
      <div className="comparison-heading">
        <div>
          <span className="eyebrow">
            <ChartNoAxesColumnIncreasing size={13} />
            EXPÉRIENCE REPRODUCTIBLE
          </span>
          <h2 id="comparison-heading">Une carte. Cinq stratégies.</h2>
          <p>
            Même carte, mêmes règles, même budget. Les cinq recherches sont exécutées à tour de
            rôle.
          </p>
        </div>
        <div className="comparison-controls">
          <label>
            Répétitions
            <select
              aria-label="Répétitions de comparaison"
              value={repetitions}
              disabled={busy || disabled}
              onChange={(event) => setRepetitions(Number(event.target.value))}
            >
              {[3, 5, 7, 11, 15].map((value) => (
                <option key={value} value={value}>
                  {value} mesures
                </option>
              ))}
            </select>
          </label>
          {busy ? (
            <button className="outline-button" onClick={onCancel}>
              <CircleStop size={16} />
              Annuler la comparaison
            </button>
          ) : (
            <button
              className="outline-button comparison-run"
              disabled={disabled}
              onClick={() => onStart(repetitions)}
            >
              <Play size={14} />
              Comparer les algorithmes
            </button>
          )}
        </div>
      </div>
      <p className="comparison-method">
        1 échauffement non mesuré par algorithme, puis {report?.repetitions ?? repetitions} mesures.
        Temps actif du moteur, sans animation ni trace d’exploration. La médiane et l’intervalle
        min–max dépendent de votre appareil.
      </p>
      <div className="comparison-status" role="status" data-testid="comparison-status">
        {busy
          ? `Comparaison en cours${progress ? ` · ${progress.completed}/${progress.total} essais traités` : ''}`
          : error
            ? 'Comparaison interrompue'
            : report
              ? report.complete
                ? 'Comparaison terminée'
                : 'Comparaison interrompue'
              : 'Prêt à comparer'}
        {report && (
          <span>
            Carte #{report.fingerprint} · {report.board.width} × {report.board.height} ·{' '}
            {report.settings.connectivity} directions
          </span>
        )}
      </div>
      {busy && (
        <progress
          max={progress?.total ?? 1}
          value={progress?.completed ?? 0}
          aria-label="Progression de la comparaison"
        />
      )}
      {error && (
        <p className="notice warning" role="alert">
          {error}
        </p>
      )}
      {report && (
        <>
          <div className="comparison-table-scroll">
            <table className="comparison-table">
              <caption className="sr-only">
                Résultats comparés sur le même instantané de carte
              </caption>
              <thead>
                <tr>
                  <th>Algorithme</th>
                  <th>Coût en pas</th>
                  <th>Surcoût¹</th>
                  <th>Développements</th>
                  <th>Frontière max.²</th>
                  <th>Calcul médian</th>
                  <th>Min – max</th>
                </tr>
              </thead>
              <tbody>
                {report.rows.map((row) => {
                  const gap = costGap(row.cost, reference);
                  return (
                    <tr
                      key={row.algorithm}
                      data-testid={`comparison-${row.algorithm}`}
                      data-status={row.status}
                      data-cost={row.cost ?? ''}
                    >
                      <th scope="row">
                        <strong>
                          {algorithms.find((item) => item.id === row.algorithm)?.label}
                        </strong>
                        <small>
                          {statuses[row.status]}
                          {row.status === 'success'
                            ? row.optimalCostGuaranteed
                              ? ' · optimal garanti'
                              : ' · sans garantie optimale'
                            : ''}
                        </small>
                      </th>
                      <td>{row.cost === null ? '—' : format(Number(row.cost) / 1000)}</td>
                      <td className={gap === 0 ? 'cost-optimal' : gap !== null ? 'cost-extra' : ''}>
                        {gap === null ? '—' : gap === 0 ? '0 %' : `+${format(gap)} %`}
                      </td>
                      <td>{row.metrics ? format(row.metrics.expandedTotal) : '—'}</td>
                      <td>{row.metrics ? format(row.metrics.frontierPeak) : '—'}</td>
                      <td>
                        <span>{duration(row.medianNs)}</span>
                        {row.medianNs !== null && (
                          <span
                            className="timing-bar"
                            aria-hidden="true"
                            style={{
                              width: `${Math.max(2, (Number(row.medianNs) / maximum) * 100)}%`,
                            }}
                          />
                        )}
                      </td>
                      <td>
                        {row.minNs === null
                          ? '—'
                          : `${duration(row.minNs)} – ${duration(row.maxNs)}`}
                        <small>
                          {row.samplesNs.length} mesure
                          {row.samplesNs.length > 1 ? 's' : ''}
                        </small>
                      </td>
                    </tr>
                  );
                })}
              </tbody>
            </table>
            {!report.rows.length && (
              <p className="comparison-empty">
                Les résultats apparaîtront après le premier algorithme mesuré.
              </p>
            )}
          </div>
          {!report.complete && !busy && (
            <p className="comparison-partial">
              Rapport incomplet : seuls les algorithmes entièrement mesurés sont conservés.
            </p>
          )}
          <div className="comparison-bottom">
            <p>
              ¹ Référence : Dijkstra lorsqu’il trouve un chemin. ² Nombre d’entrées en attente ; ce
              compteur ne mesure pas la mémoire en octets. BFS est ignoré si les coûts ne sont pas
              uniformes.
            </p>
            <div>
              <button
                className="text-button"
                disabled={busy}
                onClick={() => onExport(report, 'json')}
              >
                <ArrowDownToLine size={14} />
                Rapport JSON
              </button>
              <button
                className="text-button"
                disabled={busy}
                onClick={() => onExport(report, 'csv')}
              >
                <ArrowDownToLine size={14} />
                Tableau CSV
              </button>
            </div>
          </div>
        </>
      )}
    </section>
  );
}
