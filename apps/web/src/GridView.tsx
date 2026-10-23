import { useEffect, useRef, useState } from 'react';
import {
  Application,
  Assets,
  Container,
  Graphics,
  Rectangle,
  Sprite,
  Text,
  Texture,
} from 'pixi.js';
import 'pixi.js/unsafe-eval';
import { lineNodes } from './model';
import type { Tool } from './model';
import type { Board } from './protocol';

interface Props {
  board: Board;
  expanded: number[];
  path: number[];
  tool: Tool;
  locked: boolean;
  zoom: number;
  view: 'technical' | 'ruins';
  onEdit(nodes: number[], begin: boolean): void;
  onHover(node: number | null): void;
}
interface Scene {
  app: Application;
  base: Graphics;
  tiles: Container;
  visits: Graphics;
  route: Graphics;
  cursor: Graphics;
  labels: Container;
  cell: number;
  painted: number;
  seen: Set<number>;
}

// One shared download, four materials, sixteen patches each: detail spans four
// cells instead of shrinking an entire stone pattern into every tiny square.
const PATCHES = 4;
const TEXTURE_COUNT = 4 * PATCHES * PATCHES;
let ruinsTextures: Promise<Texture[]> | undefined;
function loadRuins() {
  return (ruinsTextures ??= (async () => {
    // Pixi's default image loader probes a blob Worker, blocked by our CSP.
    // Browser image decoding is sufficient for this single bounded atlas.
    await Assets.init({ preferences: { preferWorkers: false } });
    return Assets.load<Texture>(`${import.meta.env.BASE_URL}art/ruins-atlas.webp`);
  })()
    .then((atlas) => {
      const halfWidth = atlas.width / 2,
        halfHeight = atlas.height / 2;
      return Array.from({ length: TEXTURE_COUNT }, (_, index) => {
        const material = Math.floor(index / (PATCHES * PATCHES)),
          patch = index % (PATCHES * PATCHES);
        return new Texture({
          source: atlas.source,
          frame: new Rectangle(
            ((material % 2) + (patch % PATCHES) / PATCHES) * halfWidth,
            (Math.floor(material / 2) + Math.floor(patch / PATCHES) / PATCHES) * halfHeight,
            halfWidth / PATCHES,
            halfHeight / PATCHES,
          ),
        });
      });
    })
    .catch((error) => {
      ruinsTextures = undefined;
      throw error;
    }));
}

export function GridView(props: Props) {
  const host = useRef<HTMLDivElement>(null);
  const scene = useRef<Scene | null>(null);
  const latest = useRef(props);
  latest.current = props;
  const [loaded, setLoaded] = useState(false);
  const [failure, setFailure] = useState('');
  const [textures, setTextures] = useState<Texture[]>([]);
  const [textureError, setTextureError] = useState(false);
  const [bounds, setBounds] = useState({ width: 800, height: 520 });
  const cellSize =
    (Math.max(3, Math.min(48, bounds.width / props.board.width, bounds.height / props.board.height)) *
      props.zoom) /
    100;
  const reducedDetail = props.board.blocked.length > 16384 || cellSize < 6;
  const immersive = props.view === 'ruins' && textures.length === TEXTURE_COUNT && !reducedDetail;
  useEffect(() => {
    if (props.view !== 'ruins' || textures.length || reducedDetail) return;
    let disposed = false;
    loadRuins()
      .then((value) => {
        if (!disposed) {
          setTextures(value);
          setTextureError(false);
        }
      })
      .catch(() => {
        if (!disposed) setTextureError(true);
      });
    return () => {
      disposed = true;
    };
  }, [props.view, textures.length, reducedDetail]);

  useEffect(() => {
    const element = host.current!;
    const observer = new ResizeObserver((entries) => {
      const { width, height } = entries[0].contentRect;
      // contentRect already excludes the container's padding.
      setBounds({ width: Math.max(100, width), height: Math.max(140, height) });
    });
    observer.observe(element);
    let disposed = false;
    let app: Application | undefined;
    let initialized = false;
    const setup = async () => {
      app = new Application();
      await app.init({
        width: 800,
        height: 520,
        backgroundAlpha: 0,
        antialias: true,
        autoStart: false,
        resolution: Math.min(devicePixelRatio || 1, 2),
        autoDensity: true,
        preference: 'webgl',
        powerPreference: 'low-power',
      });
      initialized = true;
      if (disposed) {
        app.destroy(true, { children: true });
        return;
      }
      const base = new Graphics(),
        visits = new Graphics(),
        route = new Graphics(),
        cursor = new Graphics();
      const labels = new Container();
      const tiles = new Container();
      app.stage.addChild(base, tiles, visits, route, labels, cursor);
      scene.current = {
        app,
        base,
        tiles,
        visits,
        route,
        cursor,
        labels,
        cell: 20,
        painted: 0,
        seen: new Set(),
      };
      const canvas = app.canvas;
      canvas.setAttribute(
        'aria-label',
        'Grille du laboratoire. Flèches pour déplacer le curseur, espace pour dessiner.',
      );
      canvas.setAttribute('role', 'img');
      canvas.tabIndex = 0;
      element.appendChild(canvas);
      let last: number | null = null;
      let keyboardNode = latest.current.board.start;
      const locate = (event: PointerEvent) => {
        const board = latest.current.board;
        const rect = canvas.getBoundingClientRect();
        const x = Math.floor(((event.clientX - rect.left) / rect.width) * board.width);
        const y = Math.floor(((event.clientY - rect.top) / rect.height) * board.height);
        return x >= 0 && y >= 0 && x < board.width && y < board.height ? y * board.width + x : null;
      };
      const showCursor = (node: number | null) => {
        const current = scene.current;
        if (!current) return;
        current.cursor.clear();
        if (node !== null && !latest.current.locked) {
          const { width } = latest.current.board;
          const cell = current.cell;
          current.cursor
            .rect(
              (node % width) * cell + 1,
              Math.floor(node / width) * cell + 1,
              cell - 2,
              cell - 2,
            )
            .stroke({ color: 0xd4edc1, width: 2, alpha: 0.9 });
        }
        current.app.render();
        latest.current.onHover(node);
      };
      canvas.addEventListener('pointerdown', (event) => {
        if (event.button !== 0 || latest.current.locked) return;
        const node = locate(event);
        if (node === null) return;
        canvas.focus({ preventScroll: true });
        canvas.setPointerCapture(event.pointerId);
        last = node;
        latest.current.onEdit([node], true);
        showCursor(node);
      });
      canvas.addEventListener('pointermove', (event) => {
        const node = locate(event);
        showCursor(node);
        if (last !== null && node !== null && event.buttons === 1 && !latest.current.locked) {
          latest.current.onEdit(lineNodes(last, node, latest.current.board.width), false);
          last = node;
        }
      });
      const endStroke = () => {
        last = null;
      };
      canvas.addEventListener('pointerup', endStroke);
      canvas.addEventListener('pointercancel', endStroke);
      canvas.addEventListener('lostpointercapture', endStroke);
      canvas.addEventListener('pointerleave', () => showCursor(null));
      canvas.addEventListener('blur', () => showCursor(null));
      canvas.addEventListener('keydown', (event) => {
        const board = latest.current.board;
        keyboardNode = Math.min(board.blocked.length - 1, keyboardNode);
        const x = keyboardNode % board.width,
          y = Math.floor(keyboardNode / board.width);
        if (event.key === 'ArrowLeft') keyboardNode = y * board.width + Math.max(0, x - 1);
        else if (event.key === 'ArrowRight')
          keyboardNode = y * board.width + Math.min(board.width - 1, x + 1);
        else if (event.key === 'ArrowUp') keyboardNode = Math.max(0, y - 1) * board.width + x;
        else if (event.key === 'ArrowDown')
          keyboardNode = Math.min(board.height - 1, y + 1) * board.width + x;
        else if (event.key === ' ' || event.key === 'Enter') {
          if (!latest.current.locked) latest.current.onEdit([keyboardNode], true);
        } else return;
        event.preventDefault();
        showCursor(keyboardNode);
      });
      setLoaded(true);
    };
    setup().catch(() => {
      if (!disposed)
        setFailure(
          'Le rendu WebGL est indisponible. Activez l’accélération graphique de votre navigateur.',
        );
    });
    return () => {
      disposed = true;
      observer.disconnect();
      scene.current = null;
      if (initialized && app?.renderer) app.destroy(true, { children: true });
    };
  }, []);

  useEffect(() => {
    const current = scene.current;
    if (!current) return;
    const { board } = props;
    const cell = cellSize;
    current.cell = cell;
    current.app.renderer.resize(board.width * cell, board.height * cell);
    current.base.clear();
    current.visits.clear();
    current.route.clear();
    current.cursor.clear();
    current.painted = 0;
    current.seen.clear();
    for (const child of current.labels.removeChildren()) child.destroy();
    for (const child of current.tiles.removeChildren()) child.destroy();
    for (let node = 0; node < board.blocked.length; node++) {
      const x = (node % board.width) * cell,
        y = Math.floor(node / board.width) * cell;
      const weight = board.weights[node];
      const color = board.blocked[node]
        ? 0x718080
        : weight > 1
          ? weight >= 5
            ? 0x584632
            : 0x354d3c
          : 0x1d2b2a;
      if (immersive) {
        const material = board.blocked[node] ? 0 : weight >= 5 ? 3 : weight > 1 ? 2 : 1;
        const patch =
          ((node % board.width) % PATCHES) + (Math.floor(node / board.width) % PATCHES) * PATCHES;
        const sprite = new Sprite(textures[material * PATCHES * PATCHES + patch]);
        sprite.position.set(x, y);
        sprite.width = cell + 0.1;
        sprite.height = cell + 0.1;
        sprite.tint = board.blocked[node] ? 0xc0d1cb : 0xc8c9b0;
        current.tiles.addChild(sprite);
        if (board.blocked[node] && cell >= 9) {
          const edge = new Graphics()
            .rect(x + 0.5, y + 0.5, cell - 1, cell - 1)
            .stroke({ color: 0x172f2b, width: 1, alpha: 0.75 });
          current.tiles.addChild(edge);
        }
      } else
        current.base
          .roundRect(
            x + 1,
            y + 1,
            Math.max(1, cell - 2),
            Math.max(1, cell - 2),
            Math.min(2, cell * 0.08),
          )
          .fill(color);
      if (
        !board.blocked[node] &&
        weight > 1 &&
        cell >= 12 &&
        node !== board.start &&
        node !== board.goal
      ) {
        const label = new Text({
          text: String(weight),
          style: {
            fontFamily: 'monospace',
            fontSize: Math.max(8, Math.min(12, cell * 0.46)),
            fill: immersive ? 0xffffff : 0xd4bf94,
            ...(immersive ? { stroke: { color: 0x1b241d, width: 3 } } : {}),
          },
        });
        label.anchor.set(0.5);
        label.position.set(x + cell / 2, y + cell / 2);
        current.labels.addChild(label);
      }
    }
    for (const [node, color, letter] of [
      [board.start, 0xc7f488, 'D'],
      [board.goal, 0x83c7f3, 'A'],
    ] as const) {
      const x = ((node % board.width) + 0.5) * cell,
        y = (Math.floor(node / board.width) + 0.5) * cell;
      const circle = new Graphics().circle(x, y, cell * 0.4).fill(color);
      current.labels.addChild(circle);
      if (cell >= 12) {
        const label = new Text({
          text: board.start === board.goal ? '•' : letter,
          style: {
            fontFamily: 'sans-serif',
            fontSize: Math.min(15, cell * 0.5),
            fontWeight: 'bold',
            fill: 0x142222,
          },
        });
        label.anchor.set(0.5);
        label.position.set(x, y);
        current.labels.addChild(label);
      }
    }
    current.app.render();
    // Overlay is rebuilt below after a resize or map edit.
  }, [props.board, props.zoom, props.view, textures, bounds, loaded]);

  useEffect(() => {
    const current = scene.current;
    if (!current) return;
    const { board, expanded, path } = props;
    const cell = current.cell;
    if (expanded.length < current.painted) {
      current.visits.clear();
      current.seen.clear();
      current.painted = 0;
    }
    for (let index = current.painted; index < expanded.length; index++) {
      const node = expanded[index];
      if (node === board.start || node === board.goal || current.seen.has(node)) continue;
      current.seen.add(node);
      current.visits
        .rect(
          (node % board.width) * cell + 1,
          Math.floor(node / board.width) * cell + 1,
          cell - 2,
          cell - 2,
        )
        .fill({ color: 0x326a5a, alpha: immersive ? 0.72 : 1 });
    }
    current.painted = expanded.length;
    current.route.clear();
    if (path.length > 1) {
      const traceRoute = () => {
        current.route.moveTo(
          ((path[0] % board.width) + 0.5) * cell,
          (Math.floor(path[0] / board.width) + 0.5) * cell,
        );
        for (const node of path.slice(1))
          current.route.lineTo(
            ((node % board.width) + 0.5) * cell,
            (Math.floor(node / board.width) + 0.5) * cell,
          );
      };
      if (immersive) {
        traceRoute();
        current.route.stroke({
          color: 0x152c20,
          width: Math.max(4, cell * 0.35),
          alpha: 0.85,
          cap: 'round',
          join: 'round',
        });
      }
      traceRoute();
      current.route.stroke({
        color: 0xc7f488,
        width: Math.max(2, cell * 0.18),
        cap: 'round',
        join: 'round',
      });
    }
    current.app.render();
  }, [props.expanded, props.path, props.board, props.zoom, props.view, textures, bounds, loaded]);

  return (
    <div
      className={`board-scroll ${props.locked ? 'is-locked' : ''} ${props.view === 'ruins' ? 'ruins-view' : ''}`}
      ref={host}
      data-testid="grid-host"
      data-view={props.view}
      data-textures={
        textures.length === TEXTURE_COUNT ? 'ready' : textureError ? 'failed' : 'loading'
      }
    >
      {failure && (
        <p className="canvas-error" role="alert">
          {failure}
        </p>
      )}
      {!loaded && !failure && <p className="canvas-loading">Préparation de la grille…</p>}
      {props.view === 'ruins' && textureError && (
        <span className="render-note">Textures indisponibles · vue technique conservée</span>
      )}
      {props.view === 'ruins' && loaded && !reducedDetail && !textureError && textures.length === 0 && (
        <span className="render-note">Chargement des matières…</span>
      )}
      {props.view === 'ruins' && loaded && !textureError && reducedDetail && (
          <span className="render-note">Vue technique adaptée à la densité de cette carte</span>
        )}
    </div>
  );
}
