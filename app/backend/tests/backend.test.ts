import { test, expect } from '@playwright/test';

test.describe('Backend API Tests', () => {

  // Test categories endpoint
  test('Categories Endpoint', async ({ request }) => {
    console.log('Testing categories endpoint');
    const response = await request.get(`/api/categories`);
    console.log('Categories endpoint status:', response.status());

    expect(response.ok()).toBeTruthy();
    const categories = await response.json();
    console.log('Categories received:', categories);

    expect(categories).toBeDefined();
    expect(categories.length).toBeGreaterThan(0);
  });

  // Test last selected category persistence
  test('Last Selected Category Persistence', async ({ request }) => {
    console.log('Testing last selected category persistence');

    // First get available categories
    const categoriesResponse = await request.get('/api/categories');
    expect(categoriesResponse.ok()).toBeTruthy();

    const categories = await categoriesResponse.json();
    expect(categories.length).toBeGreaterThan(0);

    const existingCategory = categories[0];
    console.log(`Setting last selected category to "${existingCategory}`);

    const setCategoryResponse = await request.put('/api/last-selected-category', {
      data: { category: existingCategory }
    });

    expect(setCategoryResponse.ok()).toBeTruthy();
    const setCategoryData = await setCategoryResponse.json();
    console.log('Set category response:', setCategoryData);
  });

  // Test power endpoint
  test('Power Endpoint', async ({ request }) => {
    console.log('Testing power endpoint');
    const response = await request.get('/api/power');
    console.log('Power endpoint status:', response.status());

    expect(response.ok()).toBeTruthy();
    const power = await response.json();
    console.log('Power state:', power.isPowerOn);

    expect(power).toBeDefined();
    expect(power.isPowerOn).toBeDefined();
  });

  // Test brightness persistence
  test('Brightness Persistence', async ({ request }) => {
    console.log('Testing brightness persistence');

    const response = await request.put('/api/brightness', {
      data: { brightness: 128 }
    });

    expect(response.ok()).toBeTruthy();
    const setBrightnessData = await response.json();
    console.log('Set brightness response:', setBrightnessData);
  });

  // Test category toggle persistence
  test('Category toggle Persistence', async ({ request }) => {
    console.log('Testing category toggle persistence');

    const response = await request.put('/api/category-playback', {
      data: { enabled: true }
    });

    expect(response.ok()).toBeTruthy();
    const setToggleData = await response.json();
    console.log('Set slider response:', setToggleData);
  });
});