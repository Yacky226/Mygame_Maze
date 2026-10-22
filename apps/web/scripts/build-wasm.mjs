import { spawnSync } from 'node:child_process';
import { fileURLToPath } from 'node:url';
const repository = fileURLToPath(new URL('../../../', import.meta.url));
const destination = fileURLToPath(new URL('../public', import.meta.url));
// Paths are separate argv values: Windows spaces never enter a shell expression.
const result = spawnSync(
  'docker',
  [
    'build',
    '-f',
    'docker/Dockerfile.web',
    '--target',
    'wasm-artifacts',
    '--output',
    `type=local,dest=${destination}`,
    repository,
  ],
  { cwd: repository, stdio: 'inherit' },
);
if (result.error) console.error(result.error.message);
process.exitCode = result.status ?? 1;
