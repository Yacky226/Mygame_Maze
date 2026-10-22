import { defineConfig } from '@playwright/test';

export default defineConfig({
  testDir: './e2e',
  fullyParallel: false,
  workers: 1,
  timeout: 30000,
  expect: { timeout: 15000 },
  use: {
    baseURL: process.env.MAZE_WEB_URL || 'http://127.0.0.1:4173',
    viewport: { width: 1440, height: 1000 },
    // Repeated WebGL readbacks from trace screenshots stall software rendering
    // on small machines. Keep protocol/source traces and explicit visual checks.
    trace: { mode: 'retain-on-failure', screenshots: false, snapshots: false, sources: true },
    launchOptions: {
      args: ['--enable-unsafe-swiftshader'],
      ...(process.env.PLAYWRIGHT_CHROMIUM_EXECUTABLE_PATH
        ? { executablePath: process.env.PLAYWRIGHT_CHROMIUM_EXECUTABLE_PATH }
        : {}),
    },
  },
  webServer: process.env.MAZE_WEB_URL
    ? undefined
    : {
        command: 'npm run preview -- --port 4173',
        url: 'http://127.0.0.1:4173',
        reuseExistingServer: !process.env.CI,
      },
});
