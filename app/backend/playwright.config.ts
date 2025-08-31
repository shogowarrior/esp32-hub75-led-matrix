import { defineConfig, devices } from '@playwright/test';

// Use port 3001 for tests

export default defineConfig({
  testDir: './tests',
  fullyParallel: true,
  forbidOnly: !!process.env.CI,
  retries: process.env.CI ? 2 : 0,
  workers: process.env.CI ? 1 : undefined,
  reporter: [['list']],
  use: {
    baseURL: `http://127.0.0.1:3001`,
    trace: 'on-first-retry',
    headless: true,
  },

  // Configure the web server
  webServer: {
    // command: "PORT=3001 tsx backend.ts",
    command: "DEBUG=pw:test,pw:api,pw:webserver tsx backend.ts",
    url: "http://127.0.0.1:3001",
    reuseExistingServer: true,
    // reuseExistingServer: !process.env.CI, // Don't reuse in CI
    stdout: "pipe",
    stderr: "pipe",
    timeout: 120_1000, // 2 minutes
    env: {
      PLAYWRIGHT_TEST: 'true',
      PORT: "3001".toString()
    }
  },

  projects: [
    { name: 'chromium', use: { ...devices['Desktop Chrome'] } },
  ]
});