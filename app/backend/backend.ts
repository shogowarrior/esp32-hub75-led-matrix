const express = require('express');
const multer = require('multer');
const path = require('path');
const fs = require('fs');

// Logging middleware to track API calls
const apiLogger = (req, res, next) => {
  console.log("hello");
  const startTime = Date.now();

  // Log request details
  console.log(`\n[API CALL] ${req.method} ${req.path}`);

  // Log request parameters/body when applicable
  if (Object.keys(req.query).length > 0) {
    console.log('Query Parameters:', req.query);
  }

  if (Object.keys(req.params).length > 0) {
    console.log('URL Parameters:', req.params);
  }

  // Skip logging body for file uploads
  if (req.body && Object.keys(req.body).length > 0 && !req.path.includes('/upload')) {
    console.log('Request Body:', req.body);
  }

  // Override res.json to log response status and data
  const originalJson = res.json;
  res.json = function (body?: any) {
    const responseTime = Date.now() - startTime;
    console.log(`[RESPONSE] Status: ${res.statusCode}, Time: ${responseTime}ms`);
    if (body) {
      console.log('Response Data:', body);
    }
    return originalJson.call(this, body);
  };

  next();
};

// Initialize Express app
const app = express();
// Use environment variable for port, default to 3001 for development
const PORT = process.env.PORT ? parseInt(process.env.PORT) : 3001;


// Middleware for JSON parsing
app.use(express.json());

// Configure permissive CORS to allow all origins
app.use((req, res, next) => {
  res.header('Access-Control-Allow-Origin', '*');
  res.header('Access-Control-Allow-Methods', 'GET, POST, PUT, DELETE, OPTIONS');
  res.header('Access-Control-Allow-Headers', 'Content-Type, Authorization');

  // Handle preflight requests
  if (req.method === 'OPTIONS') {
    res.header('Access-Control-Max-Age', '86400'); // 24 hours
    return res.status(204).end();
  }

  next();
});

// Apply API logging middleware to all routes
app.use('/api', apiLogger);

// Serve static files from the gifs directory with cache control
app.use('/gifs', express.static(path.join(__dirname, 'gifs'), {
  setHeaders: (res, path) => {
    res.set('Cache-Control', 'no-store, no-cache, must-revalidate, proxy-revalidate');
    res.set('Pragma', 'no-cache');
    res.set('Expires', '0');
  }
}));

// Note: uploads directory no longer needed as we're using memory storage

// Set up multer for file uploads with memory storage
const upload = multer({
  storage: multer.memoryStorage(),
  limits: {
    fileSize: 5 * 1024 * 1024 // 5MB limit
  }
});

// Add a quick root route so Playwright can detect readiness
app.get('/', (req, res) => {
  res.send('OK'); // HTTP 200
});

// In-memory database for demo purposes
interface AppState {
  categories: string[];
  brightness: number;
  isPowerOn: boolean;
  lastSelectedCategory: string | null;
  categoryPlayback: boolean;
}

// Initialize state
let state: AppState = {
  categories: [],
  brightness: 100,
  isPowerOn: false,
  lastSelectedCategory: null,
  categoryPlayback: false
};

// File path for state persistence
const STATE_FILE_PATH = path.join(__dirname, 'state.json');

// Function to save state to file
function saveStateToFile() {
  try {
    const stateToSave = {
      ...state,
      // Don't save categories as they're loaded from the directory
      categories: undefined
    };
    fs.writeFileSync(STATE_FILE_PATH, JSON.stringify(stateToSave, null, 2));
    console.log('State saved to file');
  } catch (error) {
    console.error('Error saving state to file:', error);
  }
}

// Function to load state from file
function loadStateFromFile() {
  try {
    if (fs.existsSync(STATE_FILE_PATH)) {
      const fileContent = fs.readFileSync(STATE_FILE_PATH, 'utf-8');
      const savedState = JSON.parse(fileContent);

      // Merge saved state with default state
      state = {
        ...state,
        ...savedState,
        // Always load categories from directory
        categories: state.categories
      };

      console.log('State loaded from file');
    }
  } catch (error) {
    console.error('Error loading state from file:', error);
  }
}

// Load categories from the gifs directory
function loadCategoriesFromDirectory() {
  try {
    // Get all directories in the gifs folder (these will be our categories)
    const categoryDirs = fs.readdirSync('gifs', { withFileTypes: true })
      .filter(dirent => dirent.isDirectory())
      .map(dirent => dirent.name);

    // Add each category to the state
    state.categories = categoryDirs.map(dirName =>
      dirName.charAt(0).toUpperCase() + dirName.slice(1)
    );

    console.log(`Loaded ${state.categories.length} categories`);
  } catch (error) {
    console.error('Error loading categories from directory:', error);
    // Fallback to default data if there's an error
    state.categories = ['Reactions'];
    // Exit the process if this is a critical error
    process.exit(1);
  }
}

// Load categories when starting the server
loadCategoriesFromDirectory();

// Load saved state from file
loadStateFromFile();

// API Endpoints


// Get all categories
app.get('/api/categories', (req, res) => {
  res.json(state.categories);
});

// Get last selected category
app.get('/api/last-selected-category', (req, res) => {
  res.json({ category: state.lastSelectedCategory });
});

// Set last selected category
app.put('/api/last-selected-category', (req, res) => {
  const { category } = req.body;

  if (typeof category !== 'string') {
    return res.status(400).json({ error: 'Category must be a string' });
  }

  state.lastSelectedCategory = category;
  saveStateToFile();
  res.json({ category: state.lastSelectedCategory });
});

// Get category playback state
app.get('/api/category-playback', (req, res) => {
  res.json({ enabled: state.categoryPlayback });
});

// Set category playback state
app.put('/api/category-playback', (req, res) => {
  const { enabled } = req.body;

  if (typeof enabled !== 'boolean') {
    return res.status(400).json({ error: 'Enabled must be a boolean' });
  }

  state.categoryPlayback = enabled;
  saveStateToFile();
  res.json({ enabled: state.categoryPlayback });
});

// Play default.gif
app.post('/api/play-default', (req, res) => {
  try {
    // In a real implementation, this would trigger the ESP32 to play default.gif
    // For now, we'll just return success
    console.log('Playing default.gif');
    res.json({ success: true, message: 'Playing default.gif' });
  } catch (error) {
    console.error('Error playing default.gif:', error);
    res.status(500).json({ error: 'Failed to play default.gif' });
  }
});

// Add a new category
app.post('/api/categories', (req, res) => {
  const { name } = req.body;

  if (!name) {
    return res.status(400).json({ error: 'Category name is required' });
  }

  // Check for duplicate names
  const isDuplicate = state.categories.some(
    c => c.toLowerCase() === name.toLowerCase()
  );

  if (isDuplicate) {
    return res.status(400).json({ error: 'A category with this name already exists' });
  }

  // Create the directory for the new category
  try {
    const categoryId = name.toLowerCase().replace(/\s+/g, '-');
    const categoryDir = path.join('gifs', categoryId);

    // Create the directory if it doesn't exist
    if (!fs.existsSync(categoryDir)) {
      fs.mkdirSync(categoryDir, { recursive: true });
      console.log(`Created directory for new category: ${categoryDir}`);
    }

    // Add the category to the state
    state.categories.push(name);
    res.status(201).json(name);
  } catch (error) {
    console.error('Error creating category directory:', error);
    return res.status(500).json({ error: 'Failed to create category directory' });
  }
});

// Rename a category
app.put('/api/categories/:originalName', (req, res) => {
  const { originalName } = req.params;
  const { name } = req.body;

  if (!name) {
    return res.status(400).json({ error: 'Category name is required' });
  }

  const categoryIndex = state.categories.findIndex(c => c === originalName);
  if (categoryIndex === -1) {
    return res.status(404).json({ error: 'Category not found' });
  }

  // Check for duplicate names
  const isDuplicate = state.categories.some(
    c => c.toLowerCase() === name.toLowerCase() && c !== originalName
  );

  if (isDuplicate) {
    return res.status(400).json({ error: 'A category with this name already exists' });
  }

  try {
    // Rename the directory for the category
    const originalCategoryId = originalName.toLowerCase().replace(/\s+/g, '-');
    const newCategoryId = name.toLowerCase().replace(/\s+/g, '-');
    const originalDir = path.join('gifs', originalCategoryId);
    const newDir = path.join('gifs', newCategoryId);

    // Check if the original directory exists
    if (fs.existsSync(originalDir)) {
      // Rename the directory
      fs.renameSync(originalDir, newDir);
      console.log(`Renamed directory from ${originalDir} to ${newDir}`);
    }

    // Update the category name in state
    state.categories[categoryIndex] = name;
    res.json(name);
  } catch (error) {
    console.error('Error renaming category directory:', error);
    return res.status(500).json({ error: 'Failed to rename category directory' });
  }
});

// Delete a category
app.delete('/api/categories/:name', (req, res) => {
  const { name } = req.params;

  const categoryIndex = state.categories.findIndex(c => c === name);
  if (categoryIndex === -1) {
    return res.status(404).json({ error: 'Category not found' });
  }

  try {
    // Delete the category directory and all its contents
    const categoryId = name.toLowerCase().replace(/\s+/g, '-');
    const categoryDir = path.join('gifs', categoryId);

    // Check if the directory exists
    if (fs.existsSync(categoryDir)) {
      // Delete all files in the directory
      const files = fs.readdirSync(categoryDir);
      for (const file of files) {
        const filePath = path.join(categoryDir, file);
        fs.unlinkSync(filePath);
        console.log(`Deleted file: ${filePath}`);
      }

      // Delete the directory itself
      fs.rmdirSync(categoryDir);
      console.log(`Deleted category directory: ${categoryDir}`);
    }

    // Remove the category from state
    state.categories.splice(categoryIndex, 1);

    res.status(204).end();
  } catch (error) {
    console.error('Error deleting category directory:', error);
    return res.status(500).json({ error: 'Failed to delete category directory' });
  }
});

// Helper function to copy files
async function copyFile(inputPath: string, outputPath: string): Promise<void> {
  fs.copyFileSync(inputPath, outputPath);
  console.log('File copied:', outputPath);
}

// Upload a new GIF
app.post('/api/gifs/upload', upload.single('file'), async (req, res) => {
  try {
    // Handle file upload
    if (req.file) {
      console.log('File received:', req.file.originalname);
      console.log('Category:', req.body.category || 'none (will be saved as default.gif)');

      // Check if category is provided
      const categoryName = req.body.category;

      if (categoryName) {
        // Create the target directory if it doesn't exist
        const categoryId = categoryName.toLowerCase().replace(/\s+/g, '-');
        const targetDir = path.join('gifs', categoryId);
        if (!fs.existsSync(targetDir)) {
          fs.mkdirSync(targetDir, { recursive: true });
        }

        // Process the file based on its type
        const fileExtension = path.extname(req.file.originalname).toLowerCase();
        const fileNameWithoutExt = path.basename(req.file.originalname, fileExtension);
        let newFileName = `${Date.now()}-${fileNameWithoutExt}.gif`; // Always use .gif extension
        let newFilePath = path.join(targetDir, newFileName);

        // Check if a file with the same name already exists
        let counter = 1;
        while (fs.existsSync(newFilePath)) {
          newFileName = `${Date.now()}-${fileNameWithoutExt}-${counter}.gif`;
          newFilePath = path.join(targetDir, newFileName);
          counter++;
        }

        // Write the file buffer directly to the target location
        fs.writeFileSync(newFilePath, req.file.buffer);

        console.log(`File saved to: ${newFilePath}`);

        // If this is a new category, add it to our categories list
        if (!state.categories.some(c => c.toLowerCase() === categoryName.toLowerCase())) {
          state.categories.push(categoryName);
        }

        res.status(201).json({
          success: true,
          message: 'Image uploaded successfully to category',
          filePath: `/gifs/${categoryId}/${newFileName}`
        });
      } else {
        // If no category provided, save as default.gif
        const newFilePath = path.join('gifs', 'default.gif');

        // Write the file buffer directly to the target location
        // For default.gif, we always overwrite if it exists
        fs.writeFileSync(newFilePath, req.file.buffer);

        console.log(`File saved as default.gif: ${newFilePath}`);

        res.status(201).json({
          success: true,
          message: 'Image uploaded as default.gif',
          filePath: '/gifs/default.gif'
        });
      }
    } else {
      console.error('No file uploaded');
      return res.status(400).json({ error: 'No file uploaded' });
    }
  } catch (error) {
    console.error('Error uploading image:', error);
    res.status(500).json({ error: 'Failed to upload image' });
  }
});

// Get brightness value
app.get('/api/brightness', (req, res) => {
  res.json({ brightness: state.brightness });
});

// Set brightness value
app.put('/api/brightness', (req, res) => {
  const { brightness } = req.body;

  if (typeof brightness !== 'number' || brightness < 0 || brightness > 255) {
    return res.status(400).json({ error: 'Brightness must be a number between 0 and 255' });
  }

  state.brightness = brightness;
  saveStateToFile();
  res.json({ brightness: state.brightness });
});

// Get power state
app.get('/api/power', (req, res) => {
  res.json({ isPowerOn: state.isPowerOn });
});

// Toggle power state
app.put('/api/power', (req, res) => {
  const { isPowerOn } = req.body;

  if (typeof isPowerOn !== 'boolean') {
    return res.status(400).json({ error: 'isPowerOn must be a boolean' });
  }

  state.isPowerOn = isPowerOn;
  saveStateToFile();
  res.json({ isPowerOn: state.isPowerOn });
});


// Global error handler
process.on('uncaughtException', (error) => {
  console.error('Uncaught Exception:', error);
  console.error('Server is shutting down due to error...');
  process.exit(1);
});

process.on('unhandledRejection', (reason, promise) => {
  console.error('Unhandled Rejection at:', promise, 'reason:', reason);
  console.error('Server is shutting down due to error...');
  process.exit(1);
});

// Check if we're running in a Playwright test environment
const isPlaywrightTest = process.env.PLAYWRIGHT_TEST === 'true';

// Start the server
// const server = app.listen(PORT, () => {
const server = app.listen(PORT, '127.0.0.1', () =>  {
  console.log(`Server running on http://localhost:${PORT}`);
  // If running in Playwright test mode, log a message to indicate readiness
  if (isPlaywrightTest) {
    console.log('Server ready for Playwright tests');
  }
});

// Handle SIGTERM for graceful shutdown (used by Playwright)
process.on('SIGTERM', () => {
  console.log('Received SIGTERM, shutting down server...');
  server.close(() => {
    console.log('Server closed');
    process.exit(0);
  });
});