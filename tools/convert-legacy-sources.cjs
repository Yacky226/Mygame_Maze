// One-time, idempotent migration of the original SFML sources to UTF-8.
const fs = require('node:fs');
const path = require('node:path');
const directory = path.resolve(__dirname, '..', 'Mygame');
for (const entry of fs.readdirSync(directory, { withFileTypes: true })) {
  if (!entry.isFile() || !/\.(cpp|h)$/.test(entry.name)) continue;
  const filename = path.join(directory, entry.name);
  const data = fs.readFileSync(filename);
  try {
    new TextDecoder('utf-8', { fatal: true }).decode(data);
  } catch {
    fs.writeFileSync(filename, new TextDecoder('windows-1252').decode(data), 'utf8');
    process.stdout.write(`Converted ${entry.name}\n`);
  }
}
