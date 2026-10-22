import { test, expect } from '@playwright/test';

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
  await open(page);
  await expect(page.getByRole('heading', { name: 'Chaque chemin a sa logique.' })).toBeVisible();
  await expect(page.getByTestId('search-status')).toHaveText('Prêt à explorer');
  await page.screenshot({ path: 'test-results/studio-desktop.png', fullPage: true });
  expect(errors).toEqual([]);
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
  await expect(page.getByRole('status')).toContainText('BFS exige des coûts uniformes');
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
  await page.screenshot({ path: 'test-results/studio-mobile.png', fullPage: true });
});
