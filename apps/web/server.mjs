import { createServer } from 'node:http';
import { createReadStream } from 'node:fs';
import { stat } from 'node:fs/promises';
import path from 'node:path';
import { fileURLToPath } from 'node:url';

const root = fileURLToPath(new URL('./dist/', import.meta.url));
const mime = {
  '.txt': 'text/plain; charset=utf-8',
  '.html': 'text/html; charset=utf-8',
  '.js': 'text/javascript; charset=utf-8',
  '.mjs': 'text/javascript; charset=utf-8',
  '.wasm': 'application/wasm',
  '.css': 'text/css; charset=utf-8',
  '.svg': 'image/svg+xml',
  '.woff2': 'font/woff2',
  '.woff': 'font/woff',
  '.json': 'application/json',
};
const server = createServer(async (request, response) => {
  response.setHeader('X-Content-Type-Options', 'nosniff');
  response.setHeader('Referrer-Policy', 'no-referrer');
  response.setHeader(
    'Content-Security-Policy',
    "default-src 'self'; script-src 'self' 'wasm-unsafe-eval'; style-src 'self' 'unsafe-inline'; font-src 'self'; img-src 'self' data: blob:; worker-src 'self'; connect-src 'self'; frame-ancestors 'none'; base-uri 'self'; object-src 'none'",
  );
  if (request.method !== 'GET' && request.method !== 'HEAD') {
    response.writeHead(405, { Allow: 'GET, HEAD' }).end();
    return;
  }
  try {
    const pathname = decodeURIComponent(new URL(request.url, 'http://localhost').pathname);
    if (pathname === '/health') {
      response.writeHead(200, { 'Content-Type': 'text/plain' }).end('ok');
      return;
    }
    const filename = path.resolve(root, `.${pathname === '/' ? '/index.html' : pathname}`);
    if (!filename.startsWith(root)) {
      response.writeHead(403).end();
      return;
    }
    const info = await stat(filename);
    if (!info.isFile()) {
      response.writeHead(404).end();
      return;
    }
    response.writeHead(200, {
      'Content-Type': mime[path.extname(filename)] || 'application/octet-stream',
      'Content-Length': info.size,
      'Cache-Control': pathname.startsWith('/assets/')
        ? 'public, max-age=31536000, immutable'
        : 'no-cache',
    });
    if (request.method === 'HEAD') response.end();
    else
      createReadStream(filename)
        .on('error', () => response.destroy())
        .pipe(response);
  } catch (error) {
    response.writeHead(error instanceof URIError ? 400 : 404).end();
  }
});
server.listen(Number(process.env.PORT || 8080), process.env.HOST || '0.0.0.0', () => {
  console.log(`Maze Studio: http://localhost:${process.env.PORT || 8080}`);
});
process.on('SIGTERM', () => server.close());
process.on('SIGINT', () => server.close());
