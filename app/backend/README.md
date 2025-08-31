# ESP32 LED Matrix Backend

This is the backend server for the ESP32 LED Matrix control system.

## Running the Server

```bash
npm start
```

## Running Tests

The project uses Playwright for testing. To run the tests:

1. Make sure the backend server is running on port 3001
2. Run the tests using one of these commands:

```bash
# Run all tests
npm test

# Run tests with UI mode
npm run test:ui
```


To run tests in VSCode:

1. Install the Playwright Test for VSCode extension
2. Use the built-in Playwright test explorer to run tests

## Test Files

Test files should be placed in the `tests` directory and follow the naming convention:
- `*.spec.ts` for TypeScript tests

The tests use the Playwright testing framework and test the API endpoints of the backend server.