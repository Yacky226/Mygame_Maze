// Optional authoring utility. Production builds use the committed WebP files.
// Convert local artwork without a network request or an additional dependency.
import { chromium } from '@playwright/test';
import { readFile, writeFile } from 'node:fs/promises';
import path from 'node:path';

const [input, output] = process.argv.slice(2);
if (!input || !output || !output.endsWith('.webp') || path.resolve(input) === path.resolve(output))
  throw new Error('Usage: node scripts/optimize-art.mjs source.png destination.webp');
const bytes = await readFile(input);
const browser = await chromium.launch({ headless: true });
try {
  const page = await browser.newPage();
  const encoded = await page.evaluate(async (base64) => {
    const bytes = Uint8Array.from(atob(base64), (character) => character.charCodeAt(0));
    const bitmap = await createImageBitmap(new Blob([bytes]));
    const canvas = new OffscreenCanvas(bitmap.width, bitmap.height);
    canvas.getContext('2d').drawImage(bitmap, 0, 0);
    bitmap.close();
    const result = await canvas.convertToBlob({
      type: 'image/webp',
      quality: 0.88,
    });
    if (result.type !== 'image/webp') throw new Error('WebP encoding unavailable');
    return Array.from(new Uint8Array(await result.arrayBuffer()));
  }, bytes.toString('base64'));
  await writeFile(output, new Uint8Array(encoded));
  console.log(
    `${output}: ${encoded.length} bytes (${Math.round((encoded.length / bytes.length) * 100)}% of PNG)`,
  );
} finally {
  await browser.close();
}
