import { copyFileSync, mkdirSync } from 'node:fs';

const destination = new URL('../public/licenses/', import.meta.url);
mkdirSync(destination, { recursive: true });
for (const font of ['dm-sans', 'manrope']) {
  copyFileSync(
    new URL(`../node_modules/@fontsource-variable/${font}/LICENSE`, import.meta.url),
    new URL(`${font}-OFL.txt`, destination),
  );
}
