import { test, expect } from '@playwright/test';
import { readFile } from 'node:fs/promises';

async function open(page) {
  await page.goto('/');
  await expect(page.getByTestId('engine-state')).toHaveText('Moteur connecté');
  await expect(page.locator('canvas')).toBeVisible();
}
async function preset(page, name) {
  await page.getByRole('button', { name: 'Explorer les scénarios' }).click();
  await page
    .getByRole('dialog')
    .getByRole('button', { name: new RegExp(name) })
    .click();
}
async function run(page) {
  await page.getByRole('button', { name: 'Lancer la recherche' }).click();
}
async function fastest(page) {
  await page.getByRole('slider', { name: 'Vitesse de lecture' }).focus();
  await page.keyboard.press('End');
}

test('The real WASM engine loads and the editor renders without browser errors', async ({
  page,
}) => {
  const errors = [];
  page.on('pageerror', (error) => errors.push(error.message));
  page.on('console', (message) => {
    if (message.type() === 'error') errors.push(message.text());
  });
  await open(page);
  await expect(page.getByRole('heading', { name: 'Chaque chemin a sa logique.' })).toBeVisible();
  await expect(page.getByTestId('search-status')).toHaveText('Prêt à explorer');
  await expect(page.getByTestId('grid-host')).toHaveAttribute('data-textures', 'ready');
  await expect(page.getByRole('button', { name: 'Univers Ruines', exact: true })).toHaveAttribute(
    'aria-pressed',
    'true',
  );
  await page.screenshot({
    path: 'test-results/studio-desktop.png',
    fullPage: true,
  });
  expect(errors).toEqual([]);
});

async function exported(page, button) {
  const waiting = page.waitForEvent('download');
  await page.getByRole('button', { name: button, exact: true }).click();
  const download = await waiting;
  const filename = await download.path();
  return { filename, contents: await readFile(filename, 'utf8') };
}

async function routePixels(page) {
  const png = await page.locator('canvas').screenshot();
  return page.evaluate(async (base64) => {
    const bytes = Uint8Array.from(atob(base64), (character) => character.charCodeAt(0));
    const bitmap = await createImageBitmap(new Blob([bytes], { type: 'image/png' }));
    const canvas = new OffscreenCanvas(bitmap.width, bitmap.height),
      context = canvas.getContext('2d');
    context.drawImage(bitmap, 0, 0);
    bitmap.close();
    const pixels = context.getImageData(0, 0, canvas.width, canvas.height).data;
    let count = 0;
    for (let offset = 0; offset < pixels.length; offset += 4)
      if (pixels[offset] > 150 && pixels[offset + 1] > 210 && pixels[offset + 2] < 180) count++;
    return count;
  }, png.toString('base64'));
}

test('Seeded generators preserve provenance through editing, undo and project files', async ({
  page,
}) => {
  await open(page);
  await page.getByRole('button', { name: 'Créer un labyrinthe', exact: true }).click();
  await page.getByRole('radio', { name: /Prim/ }).check();
  await page.getByLabel('Graine de génération').fill('4294967295');
  await page.getByRole('button', { name: 'Générer la carte', exact: true }).click();
  await expect(page.getByRole('dialog')).not.toBeVisible();
  await expect(page.getByTestId('generation-info')).toContainText('Graine 4294967295');
  const first = JSON.parse((await exported(page, 'Exporter le projet')).contents);
  expect(first.board.generation).toEqual({
    algorithm: 'prim',
    seed: 4294967295,
    version: 1,
    modified: false,
  });
  await page.getByRole('button', { name: 'Créer un labyrinthe', exact: true }).click();
  await page.getByRole('button', { name: 'Générer la carte', exact: true }).click();
  await expect(page.getByRole('dialog')).not.toBeVisible();
  const repeat = await exported(page, 'Exporter le projet');
  expect(JSON.parse(repeat.contents).board).toEqual(first.board);
  await page.getByRole('button', { name: 'Gomme', exact: true }).click();
  const bounds = await page.locator('canvas').boundingBox();
  await page.mouse.click(
    bounds.x + bounds.width / (2 * first.board.width),
    bounds.y + bounds.height / (2 * first.board.height),
  );
  await expect(page.getByTestId('generation-info')).toContainText('Carte modifiée');
  await page.getByRole('button', { name: 'Annuler la modification' }).click();
  await expect(page.getByTestId('generation-info')).toContainText('Carte reproductible');
  await fastest(page);
  await run(page);
  await expect(page.getByTestId('search-status')).toHaveText('Chemin trouvé');
  await page.screenshot({
    path: 'test-results/studio-generated-ruins.png',
    fullPage: true,
  });
  await preset(page, 'Page blanche');
  await page.getByLabel('Importer un projet JSON').setInputFiles(repeat.filename);
  await expect(page.getByTestId('generation-info')).toContainText('Graine 4294967295');
});

test('The visual style can change without changing the map or search outcome', async ({ page }) => {
  const errors = [];
  page.on('pageerror', (error) => errors.push(error.message));
  await open(page);
  await preset(page, 'Le détour pondéré');
  await fastest(page);
  await expect(page.getByTestId('grid-host')).toHaveAttribute('data-textures', 'ready');
  const markers = await routePixels(page);
  const initial = JSON.parse((await exported(page, 'Exporter le projet')).contents);
  for (const style of ['Vue technique', 'Univers Ruines']) {
    await page.getByRole('button', { name: style, exact: true }).click();
    await expect(page.getByRole('button', { name: style, exact: true })).toHaveAttribute(
      'aria-pressed',
      'true',
    );
    await run(page);
    await expect(page.getByTestId('path-cost')).toHaveText('6pas');
  }
  expect(await routePixels(page)).toBeGreaterThan(markers + 200);
  expect(JSON.parse((await exported(page, 'Exporter le projet')).contents).board).toEqual(
    initial.board,
  );
  await page.getByRole('button', { name: 'Vue technique', exact: true }).click();
  const saved = await exported(page, 'Exporter le projet');
  expect(JSON.parse(saved.contents).appearance.view).toBe('technical');
  await page.getByRole('button', { name: 'Univers Ruines', exact: true }).click();
  await page.getByLabel('Importer un projet JSON').setInputFiles(saved.filename);
  await expect(page.getByRole('button', { name: 'Vue technique', exact: true })).toHaveAttribute(
    'aria-pressed',
    'true',
  );
  expect(errors).toEqual([]);
});

test('Comparison measures the same weighted snapshot and exports complete JSON and CSV reports', async ({
  page,
}) => {
  await open(page);
  await preset(page, 'Le détour pondéré');
  await page.getByLabel('Répétitions de comparaison').selectOption('3');
  await page.getByRole('button', { name: 'Comparer les algorithmes', exact: true }).click();
  await expect(page.getByTestId('comparison-status')).toContainText('Comparaison terminée');
  await expect(page.getByTestId('comparison-astar')).toHaveAttribute('data-cost', '6000');
  await expect(page.getByTestId('comparison-dijkstra')).toHaveAttribute('data-cost', '6000');
  await expect(page.getByTestId('comparison-greedy')).toHaveAttribute('data-cost', '28000');
  await expect(page.getByTestId('comparison-bfs')).toHaveAttribute(
    'data-status',
    'unsupported_configuration',
  );
  const report = JSON.parse((await exported(page, 'Rapport JSON')).contents);
  expect(report.complete).toBe(true);
  expect(report.warmups).toBe(1);
  expect(report.repetitions).toBe(3);
  expect(report.rows).toHaveLength(5);
  expect(report.board.width).toBe(5);
  for (const row of report.rows)
    expect(row.samplesNs).toHaveLength(row.algorithm === 'bfs' ? 0 : 3);
  const csv = (await exported(page, 'Tableau CSV')).contents;
  expect(csv).toContain('median_compute_active_ns');
  expect(csv).toContain('"greedy","success","28000","false"');
  await page.screenshot({
    path: 'test-results/studio-comparison.png',
    fullPage: true,
  });
  await page.getByRole('button', { name: '8 directions', exact: true }).click();
  await expect(page.getByTestId('comparison-status')).toHaveText('Prêt à comparer');
  await expect(page.getByTestId('comparison-astar')).toHaveCount(0);
});

test('A comparison can be cancelled and restarted without stale results', async ({ page }) => {
  await open(page);
  await page.getByLabel('Répétitions de comparaison').selectOption('15');
  await page.getByRole('button', { name: 'Comparer les algorithmes', exact: true }).click();
  await page.getByRole('button', { name: 'Annuler la comparaison', exact: true }).click();
  await expect(page.getByTestId('comparison-status')).toContainText('Comparaison interrompue');
  expect(JSON.parse((await exported(page, 'Rapport JSON')).contents).complete).toBe(false);
  await preset(page, 'Une arrivée isolée');
  await page.getByLabel('Répétitions de comparaison').selectOption('3');
  await page.getByRole('button', { name: 'Comparer les algorithmes', exact: true }).click();
  await expect(page.getByTestId('comparison-status')).toContainText('Comparaison terminée');
  for (const algorithm of ['astar', 'dijkstra', 'bfs', 'dfs', 'greedy'])
    await expect(page.getByTestId(`comparison-${algorithm}`)).toHaveAttribute(
      'data-status',
      'no_path',
    );
});

test('Mobile supports generation, visual styles and comparison without page overflow', async ({
  page,
}) => {
  await page.setViewportSize({ width: 390, height: 844 });
  await open(page);
  await page.getByRole('button', { name: 'Créer un labyrinthe', exact: true }).click();
  await page.getByLabel('Colonnes du labyrinthe').fill('15');
  await page.getByLabel('Lignes du labyrinthe').fill('15');
  await page.getByRole('radio', { name: /Kruskal/ }).check();
  await page.getByRole('button', { name: 'Générer la carte', exact: true }).click();
  await expect(page.getByRole('dialog')).not.toBeVisible();
  await page.getByRole('button', { name: 'Vue technique', exact: true }).click();
  await page.getByRole('button', { name: 'Comparer les algorithmes', exact: true }).click();
  await expect(page.getByTestId('comparison-status')).toContainText('Comparaison terminée');
  expect(await page.evaluate(() => document.documentElement.scrollWidth <= innerWidth)).toBe(true);
  await page.screenshot({
    path: 'test-results/studio-mobile-comparison.png',
    fullPage: true,
  });
});

test('An unavailable texture falls back to a usable technical map', async ({ page }) => {
  await page.route('**/art/ruins-atlas.webp', (route) => route.abort());
  await open(page);
  await expect(page.getByTestId('grid-host')).toHaveAttribute('data-textures', 'failed');
  await expect(page.getByText('Textures indisponibles · vue technique conservée')).toBeVisible();
  await preset(page, 'Le détour pondéré');
  await fastest(page);
  await run(page);
  await expect(page.getByTestId('path-cost')).toHaveText('6pas');
});

test('Dense maps use the technical detail level while retaining the selected visual style', async ({
  page,
}) => {
  await open(page);
  const size = 129 * 129;
  const project = {
    schemaVersion: 1,
    title: 'Grande carte',
    board: {
      width: 129,
      height: 129,
      start: 0,
      goal: size - 1,
      blocked: Array(size).fill(0),
      weights: Array(size).fill(1),
    },
    settings: {
      algorithm: 'astar',
      connectivity: 4,
      allowCornerCutting: false,
      maxExpansions: 0,
    },
    appearance: { view: 'ruins' },
  };
  await page.getByLabel('Importer un projet JSON').setInputFiles({
    name: 'dense.json',
    mimeType: 'application/json',
    buffer: Buffer.from(JSON.stringify(project)),
  });
  await expect(page.getByText('Vue technique adaptée à la densité de cette carte')).toBeVisible();
  await expect(page.getByRole('button', { name: 'Univers Ruines', exact: true })).toHaveAttribute(
    'aria-pressed',
    'true',
  );
  await fastest(page);
  await run(page);
  await expect(page.getByTestId('path-cost')).toHaveText('256pas');
});

test('Weighted routes use the native cost model and can be rerun with another algorithm', async ({
  page,
}) => {
  await open(page);
  await preset(page, 'Le détour pondéré');
  await fastest(page);
  for (const algorithm of ['astar', 'dijkstra']) {
    await page.getByLabel('Algorithme', { exact: true }).selectOption(algorithm);
    await run(page);
    await expect(page.getByTestId('search-status')).toHaveText('Chemin trouvé');
    await expect(page.getByTestId('path-cost')).toHaveText('6pas');
    await expect(page.getByTestId('path-edges')).toHaveText('6');
  }
  await run(page);
  await expect(page.getByTestId('search-status')).toHaveText('Chemin trouvé');
  await expect(page.getByTestId('path-cost')).toHaveText('6pas');
});

test('Diagonal corner policy changes reachability', async ({ page }) => {
  await open(page);
  await preset(page, 'Le coin infranchissable');
  await fastest(page);
  await run(page);
  await expect(page.getByTestId('search-status')).toHaveText('Aucun chemin possible');
  await page.getByRole('checkbox', { name: 'Couper les coins' }).check();
  await run(page);
  await expect(page.getByTestId('search-status')).toHaveText('Chemin trouvé');
  await expect(page.getByTestId('path-cost')).toContainText('1,414');
  await expect(page.getByTestId('path-edges')).toHaveText('1');
});

test('BFS refuses weighted configurations and runs on a uniform map', async ({ page }) => {
  await open(page);
  await preset(page, 'Le détour pondéré');
  await page.getByLabel('Algorithme', { exact: true }).selectOption('bfs');
  await expect(page.getByTestId('editor-notice')).toContainText('BFS exige des coûts uniformes');
  await expect(page.getByRole('button', { name: 'Lancer la recherche' })).toBeDisabled();
  await preset(page, 'Page blanche');
  await fastest(page);
  for (const algorithm of ['bfs', 'dfs']) {
    await page.getByLabel('Algorithme', { exact: true }).selectOption(algorithm);
    await run(page);
    await expect(page.getByTestId('search-status')).toHaveText('Chemin trouvé');
  }
});

test('Pause freezes progress, single-step advances, and cancellation permits a fresh run', async ({
  page,
}) => {
  await open(page);
  await page.getByRole('slider', { name: 'Vitesse de lecture' }).focus();
  await page.keyboard.press('Home');
  await run(page);
  await expect
    .poll(async () =>
      Number((await page.getByTestId('expanded-count').innerText()).replace(/\s/g, '')),
    )
    .toBeGreaterThan(1);
  await page.getByRole('button', { name: 'Pause', exact: true }).click();
  await expect(page.getByTestId('search-status')).toHaveText('Recherche en pause');
  const before = await page.getByTestId('expanded-count').innerText();
  await page.waitForTimeout(180);
  await expect(page.getByTestId('expanded-count')).toHaveText(before);
  await page.getByRole('button', { name: 'Avancer d’une étape' }).click();
  await expect
    .poll(async () =>
      Number((await page.getByTestId('expanded-count').innerText()).replace(/\s/g, '')),
    )
    .toBe(Number(before) + 1);
  await page.getByRole('button', { name: 'Annuler la recherche', exact: true }).click();
  await expect(page.getByTestId('search-status')).toHaveText('Recherche annulée');
  await preset(page, 'Le détour pondéré');
  await fastest(page);
  await run(page);
  await expect(page.getByTestId('search-status')).toHaveText('Chemin trouvé');
  await expect(page.getByTestId('path-cost')).toHaveText('6pas');
});

test('Editing, undo, and project export/import preserve the board', async ({ page }) => {
  await open(page);
  await preset(page, 'Page blanche');
  const canvas = page.locator('canvas');
  const bounds = await canvas.boundingBox();
  await page.mouse.click(bounds.x + bounds.width * 0.0125, bounds.y + bounds.height * 0.0208);
  await expect(page.getByText('1 murs', { exact: true })).toBeVisible();
  await page.keyboard.press('ControlOrMeta+z');
  await expect(page.getByText('0 murs', { exact: true })).toBeVisible();
  await page.getByRole('button', { name: 'Terrain', exact: true }).click();
  await page.getByLabel('Poids du terrain').selectOption('9');
  await page.mouse.click(bounds.x + bounds.width * 0.0125, bounds.y + bounds.height * 0.0208);
  const download = page.waitForEvent('download');
  await page.getByRole('button', { name: 'Exporter le projet' }).click();
  const file = await download;
  const saved = await file.path();
  await preset(page, 'Le détour pondéré');
  await page.getByLabel('Importer un projet JSON').setInputFiles(saved);
  await expect(page.getByText('1 terrains pondérés', { exact: true })).toBeVisible();
  await expect(page.getByText('40 × 24', { exact: false }).first()).toBeVisible();
});

test('Resource limit and no-path remain distinct outcomes', async ({ page }) => {
  await open(page);
  await fastest(page);
  await page.getByText('Budget de calcul', { exact: true }).click();
  await page.getByLabel('Maximum de développements').fill('1');
  await run(page);
  await expect(page.getByTestId('search-status')).toHaveText('Limite de calcul atteinte');
  await preset(page, 'Une arrivée isolée');
  await run(page);
  await expect(page.getByTestId('search-status')).toHaveText('Aucun chemin possible');
});

test('The editor remains usable on a narrow screen', async ({ page }) => {
  await page.setViewportSize({ width: 390, height: 844 });
  await open(page);
  await preset(page, 'Le détour pondéré');
  await fastest(page);
  await run(page);
  await expect(page.getByTestId('search-status')).toHaveText('Chemin trouvé');
  expect(await page.evaluate(() => document.documentElement.scrollWidth <= innerWidth)).toBe(true);
  await page.screenshot({
    path: 'test-results/studio-mobile.png',
    fullPage: true,
  });
});
