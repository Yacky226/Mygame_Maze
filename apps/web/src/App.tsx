import { lazy, Suspense, useEffect, useRef, useState } from 'react';
import type { ChangeEvent, ReactNode } from 'react';
import {
  ArrowDownToLine,
  ArrowUpFromLine,
  ArrowUpRight,
  BookOpen,
  Check,
  ChevronDown,
  CircleHelp,
  CircleStop,
  Eraser,
  Flag,
  Grid2X2,
  Layers,
  MapPin,
  Mountain,
  MousePointer2,
  Pause,
  Play,
  RotateCcw,
  Route,
  Shuffle,
  SkipForward,
  Sparkles,
  Square,
  Undo2,
  X,
  ZoomIn,
} from 'lucide-react';
import {
  bfsCompatible,
  blankBoard,
  defaultSettings,
  editBoard,
  parseProject,
  presetBoard,
  presetNames,
  randomBoard,
} from './model';
import type { Preset, ProjectFile, Tool } from './model';
import type {
  Algorithm,
  Board,
  SearchSettings,
  GenerationSettings,
  ComparisonReport,
} from './protocol';
import { useSearch } from './useSearch';
import { algorithms } from './algorithms';
import { generators } from './generation';
import { GeneratorForm } from './GeneratorForm';
import { ComparisonPanel } from './ComparisonPanel';
import { comparisonCsv } from './comparison';

const tools: {
  id: Tool;
  label: string;
  shortcut: string;
  icon: typeof Square;
}[] = [
  { id: 'wall', label: 'Mur', shortcut: 'M', icon: Square },
  { id: 'erase', label: 'Gomme', shortcut: 'E', icon: Eraser },
  { id: 'start', label: 'Départ', shortcut: 'D', icon: MapPin },
  { id: 'goal', label: 'Arrivée', shortcut: 'A', icon: Flag },
  { id: 'terrain', label: 'Terrain', shortcut: 'T', icon: Mountain },
];
const speedOptions = [1, 4, 16, 64, 512, 4096];
const EMPTY: number[] = [];
const GridView = lazy(() => import('./GridView').then((module) => ({ default: module.GridView })));
const number = (value: number) => new Intl.NumberFormat('fr-FR').format(value);
const time = (milliseconds: number) =>
  milliseconds < 1
    ? '< 1 ms'
    : milliseconds < 1000
      ? `${milliseconds.toFixed(milliseconds < 10 ? 2 : 0)} ms`
      : `${(milliseconds / 1000).toFixed(2)} s`;

function Modal({
  title,
  children,
  onClose,
}: {
  title: string;
  children: ReactNode;
  onClose(): void;
}) {
  const ref = useRef<HTMLDialogElement>(null);
  useEffect(() => {
    ref.current?.showModal();
  }, []);
  return (
    <dialog
      ref={ref}
      onCancel={(event) => {
        event.preventDefault();
        onClose();
      }}
      onClick={(event) => {
        if (event.target === event.currentTarget) onClose();
      }}
      aria-labelledby="modal-title"
      className="modal"
    >
      <div className="modal-heading">
        <h2 id="modal-title">{title}</h2>
        <button className="icon-button" onClick={onClose} aria-label="Fermer">
          <X size={20} />
        </button>
      </div>
      {children}
    </dialog>
  );
}

export function App() {
  const [board, setBoard] = useState<Board>(() => presetBoard('passages'));
  const [settings, setSettings] = useState<SearchSettings>(defaultSettings);
  const [title, setTitle] = useState('Les deux passages');
  const [tool, setTool] = useState<Tool>('wall');
  const [weight, setWeight] = useState(5);
  const [history, setHistory] = useState<Board[]>([]);
  const [speed, setSpeed] = useState(2);
  const [zoom, setZoom] = useState(100);
  const [hover, setHover] = useState<number | null>(null);
  const [modal, setModal] = useState<'help' | 'scenarios' | 'generator' | null>(null);
  const [view, setView] = useState<'technical' | 'ruins'>('ruins');
  const [notice, setNotice] = useState('');
  const [dimensions, setDimensions] = useState({ width: 40, height: 24 });
  const inputFile = useRef<HTMLInputElement>(null);
  const exportUrl = useRef<string | null>(null);
  useEffect(
    () => () => {
      if (exportUrl.current) URL.revokeObjectURL(exportUrl.current);
    },
    [],
  );
  const search = useSearch();
  const { active: searching, paused, result } = search;
  const active = searching || search.generating || search.comparing;
  const incompatible = settings.algorithm === 'bfs' && !bfsCompatible(board, settings);
  const algorithm = algorithms.find((item) => item.id === settings.algorithm)!;
  const walls = board.blocked.reduce((sum, blocked) => sum + blocked, 0);
  const terrain = board.weights.reduce(
    (sum, value, node) => sum + (!board.blocked[node] && value > 1 ? 1 : 0),
    0,
  );
  const path = result?.path ?? EMPTY;

  function remember() {
    setHistory((previous) => [...previous.slice(-31), board]);
  }
  function replaceBoard(next: Board, nextTitle = title) {
    remember();
    search.reset();
    setBoard(next);
    setTitle(nextTitle);
    setDimensions({ width: next.width, height: next.height });
    setNotice('');
  }
  function choosePreset(preset: Preset) {
    replaceBoard(presetBoard(preset), presetNames[preset]);
    setSettings({
      ...defaultSettings,
      connectivity: preset === 'corners' ? 8 : 4,
    });
    setZoom(100);
    setModal(null);
  }
  async function generate(config: GenerationSettings) {
    const next = await search.generate(config);
    replaceBoard(
      next,
      `${generators.find((item) => item.id === config.algorithm)!.label} · graine ${config.seed}`,
    );
    setSettings((previous) => ({
      ...previous,
      connectivity: 4,
      allowCornerCutting: false,
      maxExpansions: 0,
    }));
    setZoom(100);
    setModal(null);
  }
  function undo() {
    if (!history.length || active) return;
    const previous = history[history.length - 1];
    setHistory((items) => items.slice(0, -1));
    search.reset();
    setBoard(previous);
    setDimensions({ width: previous.width, height: previous.height });
  }
  function changeSettings(update: Partial<SearchSettings>) {
    search.reset();
    setSettings((previous) => ({ ...previous, ...update }));
  }
  function run() {
    if (!active && search.ready && !incompatible) {
      setNotice('');
      search.start(board, settings, speedOptions[speed]);
    }
  }
  function paint(nodes: number[], begin: boolean) {
    if (active) return;
    if (begin) {
      remember();
      search.reset();
    }
    setBoard((previous) => editBoard(previous, nodes, tool, weight));
  }
  function download(contents: string, mime: string, filename: string) {
    if (exportUrl.current) URL.revokeObjectURL(exportUrl.current);
    const url = URL.createObjectURL(new Blob([contents], { type: mime }));
    exportUrl.current = url;
    const link = document.createElement('a');
    link.href = url;
    link.download = filename;
    link.hidden = true;
    document.body.appendChild(link);
    link.click();
    link.remove();
    // Keep one bounded export alive until replacement/unmount: a slow download
    // must not race a fixed revocation timeout.
  }
  function exportProject() {
    const project: ProjectFile = {
      schemaVersion: 1,
      title,
      board,
      settings,
      appearance: { view },
    };
    download(JSON.stringify(project, null, 2), 'application/json', 'mon-atelier.maze-studio.json');
    setNotice('Projet exporté. Le fichier contient la carte et les paramètres de recherche.');
  }
  function exportComparison(report: ComparisonReport, format: 'json' | 'csv') {
    download(
      format === 'csv' ? comparisonCsv(report) : JSON.stringify(report, null, 2),
      format === 'csv' ? 'text/csv;charset=utf-8' : 'application/json',
      `comparaison-${report.fingerprint}.${format}`,
    );
  }
  async function importProject(event: ChangeEvent<HTMLInputElement>) {
    const file = event.target.files?.[0];
    event.target.value = '';
    if (!file) return;
    try {
      if (file.size > 2 * 1024 * 1024) throw new Error('Le fichier dépasse 2 Mio.');
      const project = parseProject(await file.text());
      replaceBoard(project.board, project.title);
      setSettings(project.settings);
      setView(project.appearance?.view ?? 'ruins');
      setZoom(100);
      setNotice('Projet importé. La carte est prête à être modifiée.');
    } catch (error) {
      setNotice(error instanceof Error ? error.message : 'Le fichier est illisible.');
    }
  }

  useEffect(() => {
    const handler = (event: KeyboardEvent) => {
      const target = event.target as HTMLElement;
      if (modal || target.closest('input,select,textarea')) return;
      if ((event.ctrlKey || event.metaKey) && event.key.toLowerCase() === 'z') {
        event.preventDefault();
        undo();
        return;
      }
      if (event.defaultPrevented || target.closest('button')) return;
      if (event.ctrlKey || event.metaKey || event.altKey) return;
      if (event.key === ' ') {
        event.preventDefault();
        if (searching) search.send(paused ? 'resume' : 'pause');
        else if (!active) run();
      } else if (!active) {
        const selected = tools.find(
          (item) => item.shortcut.toLowerCase() === event.key.toLowerCase(),
        );
        if (selected) setTool(selected.id);
      }
    };
    window.addEventListener('keydown', handler);
    return () => window.removeEventListener('keydown', handler);
  });

  let status = 'Prêt à explorer';
  if (!search.ready) status = search.error ? 'Moteur indisponible' : 'Chargement du moteur…';
  if (search.comparing) status = 'Comparaison en cours';
  else if (search.generating) status = 'Création du labyrinthe';
  else if (searching) status = paused ? 'Recherche en pause' : 'Exploration en cours';
  else if (result?.status === 'success') status = 'Chemin trouvé';
  else if (result?.status === 'no_path') status = 'Aucun chemin possible';
  else if (result?.status === 'cancelled') status = 'Recherche annulée';
  else if (result?.status === 'resource_limit') status = 'Limite de calcul atteinte';
  else if (result && result.status !== 'running') status = 'Configuration refusée';

  return (
    <div className="studio">
      <header className="topbar">
        <a className="brand" href="#atelier" aria-label="Maze Studio, atelier">
          <span className="brand-mark">
            <Route size={23} strokeWidth={2.3} />
          </span>
          <span>
            maze<span className="brand-light">studio</span>
            <span className="brand-dot">.</span>
          </span>
        </a>
        <span className="top-divider" />
        <span className="workspace-name">
          Espace personnel <ChevronDown size={14} />
        </span>
        <div className="top-actions">
          <span
            className={`engine-state ${search.ready ? 'ready' : ''}`}
            data-testid="engine-state"
          >
            <i />
            {search.ready ? 'Moteur connecté' : 'Connexion au moteur'}
          </span>
          <button
            className="text-button"
            onClick={() => inputFile.current?.click()}
            disabled={active}
          >
            <ArrowUpFromLine size={16} />
            Importer
          </button>
          <button className="outline-button" onClick={exportProject}>
            <ArrowDownToLine size={16} />
            Exporter le projet
          </button>
          <button
            className="icon-button"
            onClick={() => setModal('help')}
            aria-label="Guide de l’atelier"
          >
            <CircleHelp size={20} />
          </button>
        </div>
        <input
          ref={inputFile}
          type="file"
          accept=".json,application/json"
          className="file-input"
          onChange={importProject}
          aria-label="Importer un projet JSON"
        />
      </header>

      <aside className="sidebar">
        <div className="side-section-label">ESPACE DE TRAVAIL</div>
        <div className="side-nav active">
          <Grid2X2 size={17} />
          <span>Atelier de recherche</span>
          <span className="nav-dot" />
        </div>
        <button className="side-nav" onClick={() => setModal('scenarios')} disabled={active}>
          <Layers size={17} />
          <span>Bibliothèque de cartes</span>
          <span className="nav-count">5</span>
        </button>
        <div className="sidebar-rule" />

        <section className="side-section">
          <div className="section-heading">
            <span className="section-number">01</span>
            <h2>Algorithme</h2>
          </div>
          <label className="sr-only" htmlFor="algorithm">
            Algorithme
          </label>
          <div className="select-wrap">
            <select
              id="algorithm"
              value={settings.algorithm}
              disabled={active}
              onChange={(event) => changeSettings({ algorithm: event.target.value as Algorithm })}
            >
              {algorithms.map((item) => (
                <option key={item.id} value={item.id}>
                  {item.label}
                  {item.id === 'astar' ? ' — A star' : ''}
                </option>
              ))}
            </select>
            <ChevronDown size={15} />
          </div>
          <p className="field-note">{algorithm.note}</p>
          <span className={`guarantee ${!algorithm.optimal ? 'neutral' : ''}`}>
            <Check size={12} />
            {!algorithm.optimal
              ? 'Exploration sans garantie optimale'
              : settings.algorithm === 'bfs'
                ? 'Optimal si coûts uniformes'
                : 'Coût optimal garanti'}
          </span>
        </section>

        <section className="side-section">
          <div className="section-heading">
            <span className="section-number">02</span>
            <h2>Déplacements</h2>
          </div>
          <div className="segmented" role="group" aria-label="Voisinage">
            <button
              disabled={active}
              className={settings.connectivity === 4 ? 'selected' : ''}
              aria-pressed={settings.connectivity === 4}
              onClick={() => changeSettings({ connectivity: 4, allowCornerCutting: false })}
            >
              4 directions
            </button>
            <button
              disabled={active}
              className={settings.connectivity === 8 ? 'selected' : ''}
              aria-pressed={settings.connectivity === 8}
              onClick={() => changeSettings({ connectivity: 8 })}
            >
              8 directions
            </button>
          </div>
          <label className="switch-row">
            <span>Couper les coins</span>
            <input
              type="checkbox"
              checked={settings.allowCornerCutting}
              disabled={active || settings.connectivity !== 8}
              onChange={(event) => changeSettings({ allowCornerCutting: event.target.checked })}
            />
            <span className="switch-track" />
          </label>
          <p className="field-note">
            Un pas droit coûte 1. Une diagonale coûte 1,414, avant le poids du terrain.
          </p>
        </section>

        <section className="side-section map-section">
          <div className="section-heading">
            <span className="section-number">03</span>
            <h2>Carte</h2>
          </div>
          <div className="dimension-fields">
            <label>
              Colonnes
              <input
                aria-label="Colonnes"
                type="number"
                min="2"
                max="256"
                value={dimensions.width}
                disabled={active}
                onChange={(event) =>
                  setDimensions((previous) => ({
                    ...previous,
                    width: Number(event.target.value),
                  }))
                }
              />
            </label>
            <span>×</span>
            <label>
              Lignes
              <input
                aria-label="Lignes"
                type="number"
                min="2"
                max="256"
                value={dimensions.height}
                disabled={active}
                onChange={(event) =>
                  setDimensions((previous) => ({
                    ...previous,
                    height: Number(event.target.value),
                  }))
                }
              />
            </label>
          </div>
          <button
            className="subtle-button full-width"
            disabled={active}
            onClick={() => {
              try {
                replaceBoard(blankBoard(dimensions.width, dimensions.height), 'Nouvelle carte');
              } catch (error) {
                setNotice((error as Error).message);
              }
            }}
          >
            Créer cette grille <ArrowUpRight size={14} />
          </button>
          <button
            className="map-action"
            disabled={active}
            onClick={() => replaceBoard(randomBoard(board), 'Obstacles aléatoires')}
          >
            <Shuffle size={15} />
            Obstacles aléatoires
          </button>
          <button
            className="map-action"
            disabled={active}
            onClick={() => replaceBoard(blankBoard(board.width, board.height), 'Page blanche')}
          >
            <RotateCcw size={15} />
            Vider la carte
          </button>
          <details className="advanced">
            <summary>
              Budget de calcul <ChevronDown size={13} />
            </summary>
            <label>
              Maximum de développements
              <input
                type="number"
                min="0"
                max="1000000"
                value={settings.maxExpansions}
                disabled={active}
                onChange={(event) => {
                  const value = Number(event.target.value);
                  if (Number.isInteger(value) && value >= 0 && value <= 1000000)
                    changeSettings({ maxExpansions: value });
                }}
              />
            </label>
            <p className="field-note">0 = aucun plafond supplémentaire.</p>
          </details>
        </section>
        <div className="sidebar-bottom">
          <span className="mini-mark">
            <Route size={17} />
          </span>
          <div>
            Votre prochain chemin
            <br />
            <span>commence par une case.</span>
          </div>
        </div>
      </aside>

      <main id="atelier" className="main">
        <div className="page-heading hero-heading">
          <div>
            <div className="eyebrow">
              <span />
              LE LABORATOIRE
            </div>
            <h1>
              Chaque chemin a sa logique<span>.</span>
            </h1>
            <p>Créez des mondes. Explorez leurs chemins. Comparez vos stratégies.</p>
            <div className="hero-facts">
              <span>5 algorithmes</span>
              <i>·</i>
              <span>3 générateurs</span>
              <i>·</i>
              <span>Un moteur C++</span>
            </div>
          </div>
          <div className="heading-actions">
            <button
              className="scenario-button create-maze"
              aria-label="Créer un labyrinthe"
              onClick={() => setModal('generator')}
              disabled={active || !search.ready}
            >
              <Sparkles size={17} />
              <span>Créer un labyrinthe</span>
              <ArrowUpRight size={15} />
            </button>
            <button
              className="scenario-button"
              aria-label="Explorer les scénarios"
              onClick={() => setModal('scenarios')}
              disabled={active}
            >
              <BookOpen size={17} />
              <span>Explorer les scénarios</span>
              <ArrowUpRight size={15} />
            </button>
          </div>
        </div>

        {(notice || search.error || incompatible) && (
          <div
            className={`notice ${incompatible || search.error ? 'warning' : ''}`}
            role="status"
            data-testid="editor-notice"
          >
            <span>
              {search.error ||
                (incompatible
                  ? 'BFS exige des coûts uniformes : choisissez 4 directions et un même poids pour toutes les cases libres, ou utilisez Dijkstra / A*.'
                  : notice)}
            </span>
            {notice && !incompatible && !search.error && (
              <button
                className="icon-button"
                onClick={() => setNotice('')}
                aria-label="Masquer le message"
              >
                <X size={15} />
              </button>
            )}
          </div>
        )}

        <section className="workspace" aria-label="Éditeur et résultats">
          <div className="editor">
            <div className="editor-top">
              <div className="project-title">
                <span className="project-icon">
                  <Grid2X2 size={16} />
                </span>
                <div>
                  <strong>{title}</strong>
                  <span>
                    {board.width} × {board.height} <i>·</i> {number(board.blocked.length)} cases
                  </span>
                </div>
              </div>
              <span className="editing-state">
                <span />
                {active ? 'Lecture de la recherche' : 'Mode édition'}
              </span>
            </div>
            <div className="toolstrip">
              <div className="drawing-tools" role="group" aria-label="Outils de dessin">
                {tools.map((item) => (
                  <button
                    key={item.id}
                    aria-label={item.label}
                    disabled={active}
                    aria-pressed={tool === item.id}
                    className={tool === item.id ? 'tool selected' : 'tool'}
                    title={`${item.label} (${item.shortcut})`}
                    onClick={() => setTool(item.id)}
                  >
                    <item.icon size={16} />
                    <span>{item.label}</span>
                  </button>
                ))}
              </div>
              <div className="toolstrip-end">
                {tool === 'terrain' && (
                  <label className="weight-control">
                    Poids
                    <select
                      aria-label="Poids du terrain"
                      value={weight}
                      disabled={active}
                      onChange={(event) => setWeight(Number(event.target.value))}
                    >
                      {[2, 3, 5, 9].map((value) => (
                        <option key={value} value={value}>
                          ×{value}
                        </option>
                      ))}
                    </select>
                  </label>
                )}
                <button
                  className="icon-button"
                  onClick={undo}
                  disabled={!history.length || active}
                  title="Annuler la dernière modification (Ctrl+Z)"
                  aria-label="Annuler la modification"
                >
                  <Undo2 size={17} />
                </button>
              </div>
            </div>

            <div className="viewbar">
              <div className="view-selector" role="group" aria-label="Style de la carte">
                <button aria-pressed={view === 'technical'} onClick={() => setView('technical')}>
                  <Grid2X2 size={14} />
                  Vue technique
                </button>
                <button aria-pressed={view === 'ruins'} onClick={() => setView('ruins')}>
                  <Mountain size={14} />
                  Univers Ruines
                </button>
              </div>
              <span className="view-caption">
                {view === 'ruins' ? 'Pierre · mousse · lumière' : 'Grille · poids · exploration'}
              </span>
            </div>
            {board.generation && (
              <div className="generation-info" data-testid="generation-info">
                <Sparkles size={12} />
                {generators.find((item) => item.id === board.generation!.algorithm)?.label}
                <span>Graine {board.generation.seed}</span>
                <span>{board.generation.modified ? 'Carte modifiée' : 'Carte reproductible'}</span>
              </div>
            )}

            <Suspense
              fallback={
                <div className="board-scroll">
                  <p className="canvas-loading">Préparation de la grille…</p>
                </div>
              }
            >
              <GridView
                board={board}
                expanded={search.expanded}
                path={path}
                tool={tool}
                locked={active}
                zoom={zoom}
                view={view}
                onEdit={paint}
                onHover={setHover}
              />
            </Suspense>

            <div className="grid-footer">
              <span className="pointer-note">
                <MousePointer2 size={13} />
                {hover === null
                  ? 'Cliquez et glissez pour dessiner'
                  : `Colonne ${(hover % board.width) + 1} · Ligne ${Math.floor(hover / board.width) + 1}`}
              </span>
              <label className="zoom-control">
                <ZoomIn size={14} />
                <input
                  aria-label="Zoom de la grille"
                  type="range"
                  min="75"
                  max="175"
                  step="25"
                  value={zoom}
                  onChange={(event) => setZoom(Number(event.target.value))}
                />
                <span>{zoom}%</span>
              </label>
            </div>
            <div className="legend">
              <span>
                <i className="legend-start" />
                Départ
              </span>
              <span>
                <i className="legend-goal" />
                Arrivée
              </span>
              <span>
                <i className="legend-wall" />
                Mur
              </span>
              <span>
                <i className="legend-terrain" />
                Terrain pondéré
              </span>
              <span>
                <i className="legend-visited" />
                Exploré
              </span>
              <span>
                <i className="legend-path" />
                Chemin
              </span>
            </div>
          </div>

          <aside className="results" aria-label="Résultats de recherche">
            <div className="results-heading">
              <span>EN DIRECT</span>
              <span className={`live-dot ${active && !paused ? 'pulsing' : ''}`} />
            </div>
            <div className={`result-symbol ${result?.status === 'success' ? 'success' : ''}`}>
              <Route size={27} strokeWidth={1.5} />
            </div>
            <h2 data-testid="search-status" aria-live="polite">
              {status}
            </h2>
            <p className="result-note">
              {result?.status === 'success'
                ? result.optimalCostGuaranteed
                  ? 'Le chemin de coût minimal est surligné sur la carte.'
                  : 'Un chemin a été trouvé. Son coût peut être amélioré avec A* ou Dijkstra.'
                : result?.status === 'no_path'
                  ? 'Les obstacles empêchent de relier le départ à l’arrivée.'
                  : result?.status === 'resource_limit'
                    ? 'Le budget est épuisé. Cela ne prouve pas l’absence de chemin.'
                    : result?.status === 'cancelled'
                      ? 'Vous pouvez modifier la carte ou lancer une nouvelle recherche.'
                      : paused
                        ? 'Avancez d’une case ou reprenez l’exploration à votre rythme.'
                        : active
                          ? 'Le moteur explore les possibilités, une étape après l’autre.'
                          : 'Lancez une recherche pour observer les décisions de l’algorithme.'}
            </p>
            <div className="metric primary">
              <span>Coût du chemin</span>
              <strong data-testid="path-cost">
                {result?.cost != null ? number(Number(result.cost) / 1000) : '—'}
                <small>pas</small>
              </strong>
            </div>
            <dl className="metrics">
              <div>
                <dt>Développements</dt>
                <dd data-testid="expanded-count">
                  {result?.metrics ? number(result.metrics.expandedTotal) : '—'}
                </dd>
              </div>
              <div>
                <dt>Cases découvertes</dt>
                <dd>{result?.metrics ? number(result.metrics.discoveredUnique) : '—'}</dd>
              </div>
              <div>
                <dt>Mouvements du chemin</dt>
                <dd data-testid="path-edges">
                  {result?.status === 'success' ? number(Math.max(0, path.length - 1)) : '—'}
                </dd>
              </div>
              <div>
                <dt>Frontière maximale</dt>
                <dd>{result?.metrics ? number(result.metrics.frontierPeak) : '—'}</dd>
              </div>
              <div className="metric-separator">
                <dt title="Temps actif du moteur : hors animation, lecture du fichier et préparation de la carte.">
                  Calcul actif
                </dt>
                <dd data-testid="compute-time">
                  {result?.metrics ? time(Number(result.metrics.computeActiveNs) / 1000000) : '—'}
                </dd>
              </div>
              <div>
                <dt title="Temps depuis le lancement, y compris les pauses et l’affichage.">
                  Temps de session
                </dt>
                <dd>{result ? time(search.elapsed) : '—'}</dd>
              </div>
            </dl>
            <div className="map-summary">
              <span>
                <i />
                {number(walls)} murs
              </span>
              <span>
                <i />
                {number(terrain)} terrains pondérés
              </span>
            </div>
            <div className="results-tip">
              <Sparkles size={16} />
              <p>Le chemin le plus court en cases n’est pas toujours le moins coûteux.</p>
            </div>
          </aside>
        </section>

        <div className="playback">
          <div className="playback-actions">
            <button
              className="run-button"
              disabled={!search.ready || incompatible || search.comparing || search.generating}
              onClick={() => (searching ? search.send(paused ? 'resume' : 'pause') : run())}
            >
              {searching && !paused ? (
                <Pause size={17} fill="currentColor" />
              ) : (
                <Play size={17} fill="currentColor" />
              )}
              <span>{searching ? (paused ? 'Reprendre' : 'Pause') : 'Lancer la recherche'}</span>
              <kbd aria-hidden="true">Espace</kbd>
            </button>
            <button
              className="playback-icon"
              disabled={!searching || !paused}
              onClick={() => search.send('step')}
              aria-label="Avancer d’une étape"
              title="Avancer d’une étape"
            >
              <SkipForward size={18} />
            </button>
            <button
              className="playback-icon"
              disabled={!searching}
              onClick={() => search.send('cancel')}
              aria-label="Annuler la recherche"
              title="Annuler la recherche"
            >
              <CircleStop size={19} />
            </button>
            <span className="playback-divider" />
            <button
              className="playback-icon"
              disabled={active || !result}
              onClick={search.reset}
              aria-label="Effacer la visualisation"
              title="Effacer la visualisation"
            >
              <RotateCcw size={18} />
            </button>
          </div>
          <div className="speed-control">
            <span>Vitesse de lecture</span>
            <input
              aria-label="Vitesse de lecture"
              type="range"
              min="0"
              max="5"
              value={speed}
              onChange={(event) => {
                const value = Number(event.target.value);
                setSpeed(value);
                search.speed(speedOptions[value]);
              }}
            />
            <strong>
              {['Précise', 'Lente', 'Normale', 'Rapide', 'Très rapide', 'Max.'][speed]}
            </strong>
          </div>
        </div>
        <ComparisonPanel
          busy={search.comparing}
          disabled={!search.ready || active}
          report={search.comparison}
          progress={search.comparisonProgress}
          error={search.comparisonError}
          onStart={(repetitions) => search.compare(board, settings, repetitions)}
          onCancel={() => search.send('cancel')}
          onExport={exportComparison}
        />
        <footer className="main-footer">
          <span>
            <span className="footer-dot" />
            {search.ready ? `Moteur ${search.version} prêt` : 'Initialisation'}
            <i>·</i>Carte traitée dans votre navigateur
          </span>
          <span>
            MAZE STUDIO <span className="footer-version">/ 0.3</span>
          </span>
        </footer>
      </main>

      {modal === 'generator' && (
        <Modal
          title="Construire un nouveau monde"
          onClose={() => {
            if (!search.generating) setModal(null);
          }}
        >
          <GeneratorForm
            busy={search.generating}
            initial={{
              width: board.width >= 3 && board.width % 2 === 1 ? board.width : 41,
              height: board.height >= 3 && board.height % 2 === 1 ? board.height : 25,
              algorithm: board.generation?.algorithm ?? 'backtracker',
              seed: board.generation?.seed ?? 42,
            }}
            onGenerate={generate}
          />
        </Modal>
      )}
      {modal === 'help' && (
        <Modal title="Prendre en main l’atelier" onClose={() => setModal(null)}>
          <div className="help-content">
            <p>
              Choisissez un outil, puis cliquez ou glissez sur la grille. Les murs bloquent le
              passage ; le terrain multiplie le coût d’entrée dans une case.
            </p>
            <ol>
              <li>Placez le départ et l’arrivée avec les outils dédiés.</li>
              <li>Choisissez un algorithme et un voisinage à 4 ou 8 directions.</li>
              <li>
                Lancez la recherche. Pause, pas à pas et vitesse contrôlent la progression du
                moteur.
              </li>
              <li>Comparez le coût, les développements et le calcul actif.</li>
            </ol>
            <p>
              <strong>Raccourcis :</strong> M : mur, E : gomme, D : départ, A : arrivée, T :
              terrain. Espace : lancer ou pause. Ctrl+Z : annuler un dessin. Sur la grille au
              clavier : flèches, puis espace pour appliquer l’outil.
            </p>
            <p>
              Le calcul actif exclut la lecture animée et la préparation de la recherche. Le temps
              de session inclut les pauses. BFS refuse les coûts non uniformes ; DFS ne garantit pas
              le coût minimal. Greedy privilégie la proximité de l’arrivée, sans garantie optimale.
            </p>
            <p>
              Créez un labyrinthe avec une graine pour le reproduire. La vue technique et l’univers
              Ruines représentent exactement la même carte. Le comparateur mesure les cinq
              algorithmes sans animation et exporte un rapport JSON ou CSV.
            </p>
            <p>
              Exportez pour conserver votre travail. La carte reste en mémoire dans l’onglet
              jusqu’au rechargement.
            </p>
          </div>
        </Modal>
      )}
      {modal === 'scenarios' && (
        <Modal title="Une carte, une question" onClose={() => setModal(null)}>
          <p className="modal-intro">
            Cinq points de départ pour explorer le comportement des algorithmes.
          </p>
          <div className="scenario-list">
            {(Object.keys(presetNames) as Preset[]).map((preset, index) => (
              <button key={preset} onClick={() => choosePreset(preset)}>
                <span className="scenario-index">0{index + 1}</span>
                <span>
                  <strong>{presetNames[preset]}</strong>
                  <small>
                    {
                      {
                        passages: 'Contourner les murs ou traverser le terrain coûteux ?',
                        weighted: 'Un détour de six pas coûte moins que la ligne droite.',
                        blank: 'Un espace libre pour construire votre expérience.',
                        corners: 'Deux cases libres, mais un angle qui change tout.',
                        'no-path': 'Savoir conclure qu’aucun passage n’existe.',
                      }[preset]
                    }
                  </small>
                </span>
                <ArrowUpRight size={17} />
              </button>
            ))}
          </div>
        </Modal>
      )}
    </div>
  );
}
