// All image resizing is done in the frontend to ensure consistent 64x64 size
interface AppState {
  categories: string[];
  categorySelected: string;
  isPowerOn: boolean;
  brightness: number;
  categoryPlayback: boolean;
  isLoading: boolean;
  lastSelectedCategory: string | null;
}

// API Configuration
const API_BASE_URL = 'http://localhost:3001';

// API Client
const apiClient = {
  async get(endpoint: string): Promise<any> {
    try {
      const response = await fetch(`${API_BASE_URL}${endpoint}`);
      if (!response.ok) {
        throw new Error(`HTTP error! status: ${response.status}`);
      }
      return await response.json();
    } catch (error) {
      console.error(`Error fetching ${endpoint}:`, error);
      throw error;
    }
  },

  async post(endpoint: string, data: any): Promise<any> {
    try {
      const response = await fetch(`${API_BASE_URL}${endpoint}`, {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json',
        },
        body: JSON.stringify(data),
      });

      // Try to parse the response body even if status is not OK
      const responseData = await response.json();

      if (!response.ok) {
        // If the response contains an error message, use that
        if (responseData && responseData.error) {
          throw new Error(responseData.error);
        }
        // Otherwise use the default error message
        throw new Error(`HTTP error! status: ${response.status}`);
      }

      return responseData;
    } catch (error) {
      console.error(`Error posting to ${endpoint}:`, error);
      throw error;
    }
  },

  async put(endpoint: string, data: any): Promise<any> {
    try {
      const response = await fetch(`${API_BASE_URL}${endpoint}`, {
        method: 'PUT',
        headers: {
          'Content-Type': 'application/json',
        },
        body: JSON.stringify(data),
      });
      if (!response.ok) {
        throw new Error(`HTTP error! status: ${response.status}`);
      }
      return await response.json();
    } catch (error) {
      console.error(`Error putting to ${endpoint}:`, error);
      throw error;
    }
  },

  async delete(endpoint: string): Promise<void> {
    try {
      const response = await fetch(`${API_BASE_URL}${endpoint}`, {
        method: 'DELETE',
      });
      if (!response.ok) {
        throw new Error(`HTTP error! status: ${response.status}`);
      }
    } catch (error) {
      console.error(`Error deleting ${endpoint}:`, error);
      throw error;
    }
  },

  async uploadFile(endpoint: string, formData: FormData): Promise<any> {
    try {
      const response = await fetch(`${API_BASE_URL}${endpoint}`, {
        method: 'POST',
        body: formData,
      });
      if (!response.ok) {
        const errorData = await response.json();
        throw new Error(errorData.error || `HTTP error! status: ${response.status}`);
      }
      return await response.json();
    } catch (error) {
      console.error(`Error uploading to ${endpoint}:`, error);
      throw error;
    }
  }
};

// Initialize state
const state: AppState = {
  categories: [],
  categorySelected: '',
  isPowerOn: false,
  brightness: 100,
  categoryPlayback: false,
  isLoading: false,
  lastSelectedCategory: null
};

// DOM Elements
const categoryList = document.getElementById('category-list') as HTMLElement | null;
const addCategoryBtn = document.getElementById('add-category-btn') as HTMLButtonElement | null;
const addCategoryModal = document.getElementById('add-category-modal') as HTMLElement | null;
const confirmAddCategoryBtn = document.getElementById('confirm-add-category') as HTMLButtonElement | null;
const newCategoryInput = document.getElementById('new-category-name') as HTMLInputElement | null;
const renameCategoryModal = document.getElementById('rename-category-modal') as HTMLElement | null;
const confirmRenameCategoryBtn = document.getElementById('confirm-rename-category') as HTMLButtonElement | null;
const renameCategoryInput = document.getElementById('rename-category-input') as HTMLInputElement | null;
const deleteConfirmationModal = document.getElementById('delete-confirmation-modal') as HTMLElement | null;
const confirmDeleteCategoryBtn = document.getElementById('confirm-delete-category') as HTMLButtonElement | null;
const cancelDeleteCategoryBtn = document.getElementById('cancel-delete-category') as HTMLButtonElement | null;
const deleteConfirmationText = document.getElementById('delete-confirmation-text') as HTMLElement | null;
const errorMessage = document.getElementById('error-message') as HTMLElement | null;
const successMessage = document.getElementById('success-message') as HTMLElement | null;
const categorySelect = document.getElementById('category-select') as HTMLSelectElement | null;
const brightnessSlider = document.getElementById('brightness-slider') as HTMLInputElement | null;
const powerBtn = document.getElementById('power-btn') as HTMLButtonElement | null;
const categoryPlaybackToggle = document.getElementById('category-playback-toggle') as HTMLInputElement | null;
const loadingElement = document.getElementById('loading') as HTMLElement | null;
const uploadModal = document.getElementById('upload-modal') as HTMLElement | null;
const showUploadModalBtn = document.getElementById('show-upload-modal') as HTMLButtonElement | null;
const closeUploadModalBtn = document.getElementById('close-upload-modal') as HTMLButtonElement | null;
const fileUploadBtn = document.getElementById('file-upload-btn') as HTMLButtonElement | null;
const fileUpload = document.getElementById('file-upload') as HTMLInputElement | null;
const uploadBtn = document.getElementById('upload-btn') as HTMLButtonElement | null;

// Add Category Modal Upload Elements
const fileUploadCategory = document.getElementById('file-upload-category') as HTMLInputElement | null;
const fileUploadCategoryBtn = document.getElementById('file-upload-category-btn') as HTMLButtonElement | null;
const uploadCategoryBtn = document.getElementById('upload-category-btn') as HTMLButtonElement | null;

// Add CSS styles for category actions
function addCategoryStyles() {
  // First check if frontend.css is already loaded
  const existingStylesheet = document.querySelector('link[href="frontend.css"]');
  if (!existingStylesheet) {
    // If not loaded, create a link to the CSS file
    const link = document.createElement('link');
    link.rel = 'stylesheet';
    link.href = 'frontend.css';
    document.head.appendChild(link);
  }

  const style = document.createElement('style');
  style.textContent = `
    /* Hide category actions by default */
    .category-actions {
      display: none;
    }

    /* Show category actions on hover or when active */
    .category-item:hover .category-actions,
    .category-item.active .category-actions {
      display: flex;
    }

    /* Prevent category item from shifting when hovered */
    .category-item {
      transition: none;
      transform: none;
      position: relative;
      min-height: 20px; /* Reduced height to make items more compact */
      display: flex;
      align-items: center;
      padding-left: 10px; /* Match the spacing on both sides */
    }

    /* Ensure category name doesn't move when actions appear */
    .category-name {
      display: inline-block;
      vertical-align: middle;
      flex-grow: 1;
      margin-right: 10px; /* Space for action buttons */
      padding-left: 5px; /* Small padding between edge and name */
    }

    /* Style for action buttons container */
    .category-actions {
      position: absolute;
      right: 0;
      top: 50%;
      transform: translateY(-50%);
      display: none;
      gap: 5px;
      padding-right: 10px; /* Match the spacing on both sides */
    }

    /* Highlight selected category */
    .category-item.selected {
      background-color: rgba(0, 0, 0, 0.1);
      border-left: 2px solid #4CAF50;
      padding-left: 8px; /* Adjust for border */
    }

    /* Grey out categories when disabled */
    .category-item.disabled {
      opacity: 0.5;
      pointer-events: none;
      cursor: not-allowed;
    }
  `;
  document.head.appendChild(style);
}

// Initialize the app
async function init() {
  try {
    console.log('Initializing app...');

    // Add category styles
    addCategoryStyles();

    // Show loading indicator
    if (loadingElement) loadingElement.style.display = 'block';
    if (loadingElement) loadingElement.textContent = 'Loading...';

    // Check if backend is available
    try {
      await checkBackendAvailability();
    } catch (error) {
      console.error('Backend not available:', error);
      state.isLoading = false;
      showError('Backend not available. Showing loading state. Will update when backend is available.');
      if (loadingElement) loadingElement.textContent = 'Backend not available. Waiting for connection...';

      // Start periodic check for backend availability
      const checkInterval = setInterval(async () => {
        try {
          await checkBackendAvailability();
          clearInterval(checkInterval);
          // Reload data once backend is available
          await loadDataFromAPI();
          updateUI();
        } catch (error) {
          // Backend still not available, continue checking
        }
      }, 5000); // Check every 5 seconds

      return;
    }

    // Backend is available, load data
    await loadDataFromAPI();

    // Load last selected category from API
    try {
      console.log('Loading last selected category from API...');
      await loadLastSelectedCategoryFromAPI();
    } catch (error) {
      console.error('Error loading last selected category from API:', error);
      showError('Failed to load last selected category. Using default value.');
    }

    // Load category playback state from API
    try {
      console.log('Loading category playback state from API...');
      await loadCategoryPlaybackStateFromAPI();
    } catch (error) {
      console.error('Error loading category state from API:', error);
      showError('Failed to load category state. Using default value.');
    }

    // Set up UI
    setupEventListeners();

    // Ensure we have a valid selected category
    if (state.categories.length > 0 && !state.categorySelected) {
      state.categorySelected = state.categories[0];
    }

    updateCategoryList();
    updateUI();

    // Load brightness value from API
    try {
      console.log('Loading brightness from API...');
      await loadBrightnessFromAPI();
    } catch (error) {
      console.error('Error loading brightness:', error);
      showError('Failed to load brightness. Using default value.');
      // Use default brightness if API fails
      state.brightness = 100;
      if (brightnessSlider) brightnessSlider.value = state.brightness.toString();
    }

    // Load power state from API
    try {
      console.log('Loading power state from API...');
      await loadPowerStateFromAPI();
    } catch (error) {
      console.error('Error loading power state:', error);
      showError('Failed to load power state. Using default value.');
      // Use default power state if API fails
      state.isPowerOn = false;
    }

    console.log('App initialized successfully with data from API');
  } catch (error) {
    console.error('Error initializing app:', error);
    console.error('Error details:', error instanceof Error ? error.message : String(error));
    if (error instanceof Object && 'response' in error) {
      const err = error as { response: { status: number, data: any } };
      console.error('Response status:', err.response.status);
      console.error('Response data:', err.response.data);
    }
    // Show a more detailed error message to help with debugging
    showError(`Failed to load data from server: ${error instanceof Error ? error.message : String(error)}. Check console for details.`);
  } finally {
    // Hide loading indicator
    if (loadingElement) {
      loadingElement.style.display = 'none';
    }
  }
}

// Check if backend is available
async function checkBackendAvailability(): Promise<void> {
  try {
    const response = await fetch(`${API_BASE_URL}/api/categories`, {
      method: 'GET',
      cache: 'no-store'
    });

    if (!response.ok) {
      throw new Error(`Backend returned status: ${response.status}`);
    }

    state.isLoading = false;
  } catch (error) {
    console.error('Backend check failed:', error);
    state.isLoading = false;
    throw error;
  }
}

// Load last selected category from API
async function loadLastSelectedCategoryFromAPI(): Promise<void> {
  try {
    console.log('Fetching last selected category from /api/last-selected-category...');
    const response = await apiClient.get('/api/last-selected-category');
    console.log('Last selected category received:', response.category);

    if (response.category && state.categories.includes(response.category)) {
      state.categorySelected = response.category;
      state.lastSelectedCategory = response.category;
      console.log(`Restored last selected category: ${response.category}`);
    } else if (state.categories.length > 0) {
      // If no valid last selected category, select the first one
      state.categorySelected = state.categories[0];
      console.log(`No valid last selected category, using first category: ${state.categorySelected}`);
    }
  } catch (error) {
    console.error('Error loading last selected category from API:', error);
    console.error('Error details:', error instanceof Error ? error.message : String(error));
    if (error instanceof Object && 'response' in error) {
      const err = error as { response: { status: number, data: any } };
      console.error('Response status:', err.response.status);
      console.error('Response data:', err.response.data);
    }
    throw error;
  }
}

// Load data from API
async function loadDataFromAPI(): Promise<void> {
  try {
    console.log('Attempting to load data from API...');
    state.isLoading = true;

    // Fetch categories
    console.log('Fetching categories from /api/categories...');
    const categories = await apiClient.get('/api/categories') as string[];
    console.log('Categories received:', categories);
    state.categories = categories;

    // Select the first category if available
    if (state.categories.length > 0) {
      state.categorySelected = state.categories[0];
    }

    state.isLoading = false;
    console.log('Successfully loaded data from API');
  } catch (error) {
    state.isLoading = false;
    console.error('Error loading data from API:', error);
    console.error('Error details:', error instanceof Error ? error.message : String(error));
    if (error instanceof Object && 'response' in error) {
      const err = error as { response: { status: number, data: any } };
      console.error('Response status:', err.response.status);
      console.error('Response data:', err.response.data);
    }
    throw error;
  }
}

// Load brightness from API
async function loadBrightnessFromAPI(): Promise<void> {
  try {
    console.log('Fetching brightness from /api/brightness...');
    const response = await apiClient.get('/api/brightness');
    console.log('Brightness received:', response.brightness);
    state.brightness = response.brightness;
    if (brightnessSlider) brightnessSlider.value = state.brightness.toString();
  } catch (error) {
    console.error('Error loading brightness from API:', error);
    console.error('Error details:', error instanceof Error ? error.message : String(error));
    if (error instanceof Object && 'response' in error) {
      const err = error as { response: { status: number, data: any } };
      console.error('Response status:', err.response.status);
      console.error('Response data:', err.response.data);
    }
    throw error;
  }
}

// Load power state from API
async function loadPowerStateFromAPI(): Promise<void> {
  try {
    console.log('Fetching power state from /api/power...');
    const response = await apiClient.get('/api/power');
    console.log('Power state received:', response.isPowerOn);
    state.isPowerOn = response.isPowerOn;
  } catch (error) {
    console.error('Error loading power state from API:', error);
    console.error('Error details:', error instanceof Error ? error.message : String(error));
    if (error instanceof Object && 'response' in error) {
      const err = error as { response: { status: number, data: any } };
      console.error('Response status:', err.response.status);
      console.error('Response data:', err.response.data);
    }
    throw error;
  }
}

// Load category playback state from API
async function loadCategoryPlaybackStateFromAPI(): Promise<void> {
  try {
    console.log('Fetching category playback state from /api/category-playback...');
    const response = await apiClient.get('/api/category-playback');
    console.log('Category playback state received:', response.enabled);
    state.categoryPlayback = response.enabled;

    // Update UI
    if (categoryPlaybackToggle) {
      categoryPlaybackToggle.checked = state.categoryPlayback;
    }

    // If enabling categories, resume the last selected category
    if (state.categoryPlayback && state.lastSelectedCategory) {
      try {
        console.log(`Resuming last selected category: ${state.lastSelectedCategory}`);
        await selectCategory(state.lastSelectedCategory);
      } catch (error) {
        console.error('Error resuming last selected category:', error);
        // If there's an error, just continue with the current state
      }
    }

    // Update category list to reflect the correct enabled/disabled state
    updateCategoryList();
  } catch (error) {
    console.error('Error loading category enabled state from API:', error);
    console.error('Error details:', error instanceof Error ? error.message : String(error));
    if (error instanceof Object && 'response' in error) {
      const err = error as { response: { status: number, data: any } };
      console.error('Response status:', err.response.status);
      console.error('Response data:', err.response.data);
    }
    throw error;
  }
}

// Set up event listeners
function setupEventListeners(): void {
  // Category events
  console.log('Setting up event listeners...');
  addCategoryBtn?.addEventListener('click', () => addCategoryModal && (addCategoryModal.style.display = 'block'));
  confirmAddCategoryBtn?.addEventListener('click', addCategory);
  confirmRenameCategoryBtn?.addEventListener('click', renameCategory);

  // Make sure delete confirmation button has the event listener
  if (confirmDeleteCategoryBtn) {
    console.log('Setting up delete confirmation button click handler');
    confirmDeleteCategoryBtn.onclick = function() {
      console.log('Delete confirmation button clicked');
      deleteCategory();
    };
  } else {
    console.log('confirmDeleteCategoryBtn is null or undefined');
  }

  cancelDeleteCategoryBtn?.addEventListener('click', () => {
    console.log('Cancel delete button clicked');
    deleteConfirmationModal && (deleteConfirmationModal.style.display = 'none');
  });

  // Modal close buttons
  document.querySelectorAll('.close-modal').forEach(btn => {
    btn.addEventListener('click', () => {
      const modalId = (btn as HTMLElement).getAttribute('data-modal');
      if (modalId) {
        const modal = document.getElementById(modalId) as HTMLElement | null;
        if (modal) modal.style.display = 'none';
      }
    });
  });

  // Upload modal
  showUploadModalBtn?.addEventListener('click', () => {
    console.log('Show upload modal button clicked');
    uploadModal && (uploadModal.style.display = 'block');
  });
  closeUploadModalBtn?.addEventListener('click', () => {
    console.log('Close upload modal button clicked');
    uploadModal && (uploadModal.style.display = 'none');
  });

  // Create a hidden file input element for the upload modal
  const hiddenFileInput = document.createElement('input');
  hiddenFileInput.type = 'file';
  hiddenFileInput.id = 'file-upload';
  hiddenFileInput.accept = 'image/*';
  hiddenFileInput.style.display = 'none';
  document.body.appendChild(hiddenFileInput);

  // Update the fileUpload reference to point to our new hidden input
  const fileUpload = hiddenFileInput;
  const fileUploadBtn = document.getElementById('file-upload-btn') as HTMLButtonElement | null;
  const categorySelect = document.getElementById('category-select') as HTMLSelectElement | null;
  const uploadTypeToggle = document.getElementById('upload-type-toggle') as HTMLInputElement | null;

  // Set up the upload type toggle to enable/disable category selection
  if (uploadTypeToggle && categorySelect) {
    uploadTypeToggle.addEventListener('change', () => {
      categorySelect.disabled = !uploadTypeToggle.checked;
    });

    // Initialize the disabled state based on the toggle
    categorySelect.disabled = !uploadTypeToggle.checked;
  }

  // Set up the Choose File button
  fileUploadBtn?.addEventListener('click', () => {
    console.log('File upload button clicked');
    fileUpload.click();
  });

  // Set up the upload button to handle upload
  const uploadBtnElement = document.getElementById('upload-btn');
  if (uploadBtnElement) {
    console.log('Found upload button element, attaching event listener');
    uploadBtnElement.addEventListener('click', () => {
      console.log('Upload button clicked, calling handleUpload');
      handleUpload();
    });

    // Add a visual indicator that the button is clickable
    uploadBtnElement.style.cursor = 'pointer';
  } else {
    console.error('Upload button element not found!');
  }

  // Create a test button for debugging
    const testUploadBtn = document.createElement('button');
    testUploadBtn.textContent = 'Test Upload';
    testUploadBtn.style.position = 'fixed';
    testUploadBtn.style.bottom = '20px';
    testUploadBtn.style.right = '20px';
    testUploadBtn.style.zIndex = '1000';
    testUploadBtn.style.padding = '10px';
    testUploadBtn.style.backgroundColor = 'red';
    testUploadBtn.style.color = 'white';
    testUploadBtn.style.border = 'none';
    testUploadBtn.style.borderRadius = '5px';
    testUploadBtn.style.cursor = 'pointer';

    testUploadBtn.addEventListener('click', async () => {
      console.log('Test upload button clicked');

      // Create a 64x64 test image with a red square
      const canvas = document.createElement('canvas');
      canvas.width = 64;
      canvas.height = 64;
      const ctx = canvas.getContext('2d');
      if (ctx) {
        ctx.fillStyle = 'transparent';
        ctx.fillRect(0, 0, 64, 64);
        ctx.fillStyle = 'red';
        ctx.fillRect(16, 16, 32, 32); // Centered red square
      }

      // Get the file input element directly
      const fileInput = document.getElementById('file-upload') as HTMLInputElement | null;
      if (!fileInput) {
        console.error('File input element not found');
        return;
      }

      // Create a test file and trigger the upload
      canvas.toBlob(async (blob) => {
        if (blob) {
          const testFile = new File([blob], 'test-64x64.gif', { type: 'image/gif' });

          // Create a data transfer to simulate file selection
          const dataTransfer = new DataTransfer();
          dataTransfer.items.add(testFile);
          fileInput.files = dataTransfer.files;

          // Get the category select element directly
          const categorySelect = document.getElementById('category-select') as HTMLSelectElement | null;
          if (categorySelect) {
            // Clear any selected category to ensure upload to default.gif
            categorySelect.value = '';
          }

          // Call handleUpload directly with the test file
          try {
            // Show loading indicator
            if (loadingElement) loadingElement.style.display = 'block';

            // Prepare the image for upload
            console.log('Preparing image for upload...');
            const preparedFile = await prepareImageForUpload(testFile);

            // Prepare form data for file upload
            const formData = new FormData();
            formData.append('file', preparedFile);

            // Call API to upload image
            console.log('Uploading test image as default.gif...');
            const response = await apiClient.uploadFile('/api/gifs/upload', formData);
            console.log('Upload response:', response);

            // Show success message
            showSuccess('64x64 test image uploaded as default.gif successfully!');

            // Reset form
            if (fileInput) fileInput.value = '';
          } catch (error) {
            console.error('Error uploading test image:', error);
            showError(error instanceof Error ? error.message : 'Failed to upload test image');
          } finally {
            // Hide loading indicator
            if (loadingElement) loadingElement.style.display = 'none';
          }
        }
      }, 'image/gif');
    });

  document.body.appendChild(testUploadBtn);

  // Set up the file input to update the UI when a file is selected
  fileUpload.addEventListener('change', (event) => {
    console.log('File input changed, file selected');
    const file = (event.target as HTMLInputElement).files?.[0];
    if (file) {
      console.log('File selected:', file.name);
      // Remove any existing file name display or preview
      const existingFileName = document.getElementById('selected-file-name');
      if (existingFileName) existingFileName.remove();

      const existingPreview = document.querySelector('.image-preview');
      if (existingPreview) existingPreview.remove();

      // Show the selected file name
      const fileNameDisplay = document.createElement('div');
      fileNameDisplay.id = 'selected-file-name';
      fileNameDisplay.textContent = `Selected: ${file.name}`;
      fileNameDisplay.style.margin = '10px 0';
      fileNameDisplay.style.fontSize = '12px';
      fileNameDisplay.style.color = 'var(--text-color)';

      // Create image preview
      const previewContainer = document.createElement('div');
      previewContainer.className = 'image-preview';

      const reader = new FileReader();
      reader.onload = function(e) {
        const img = document.createElement('img');
        img.src = e.target?.result as string;
        img.alt = 'Preview';
        previewContainer.appendChild(img);
      };
      reader.readAsDataURL(file);

      // Insert the elements before the upload buttons
      const uploadButtons = document.querySelector('.upload-buttons');
      if (uploadButtons) {
        uploadButtons.parentNode?.insertBefore(previewContainer, uploadButtons);
        uploadButtons.parentNode?.insertBefore(fileNameDisplay, uploadButtons);
      }
    } else {
      console.log('No file selected or file selection cancelled');
    }
  });

  // Add Category Modal Upload
  fileUploadCategoryBtn?.addEventListener('click', () => fileUploadCategory?.click());
  uploadCategoryBtn?.addEventListener('click', handleUploadFromCategoryModal);

  // Brightness control
  brightnessSlider?.addEventListener('input', async (e) => {
    const brightnessValue = parseInt((e.target as HTMLInputElement).value);
    state.brightness = brightnessValue;
    console.log(`Brightness set to: ${state.brightness}`);

    try {
      // Update brightness on the server
      await apiClient.put('/api/brightness', { brightness: brightnessValue });
    } catch (error) {
      console.error('Error updating brightness:', error);
      showError('Failed to update brightness');
    }
  });

  // Power button
  powerBtn?.addEventListener('click', togglePower);

  // Category playback toggle
  categoryPlaybackToggle?.addEventListener('change', toggleCategoryPlayback);
}

// Update UI based on state
function updateUI(): void {
  // Update power button
  if (powerBtn) {
    if (state.isPowerOn) {
      powerBtn.textContent = 'ON';
      powerBtn.classList.add('on');
      powerBtn.classList.remove('off');
    } else {
      powerBtn.textContent = 'OFF';
      powerBtn.classList.add('off');
      powerBtn.classList.remove('on');
    }
  }

  // Update category playback toggle
  if (categoryPlaybackToggle) {
    categoryPlaybackToggle.checked = state.categoryPlayback;
  }
}

// Toggle power state
async function togglePower(): Promise<void> {
  const newPowerState = !state.isPowerOn;

  try {
    // Update power state on the server
    console.log(`Updating power state to ${newPowerState}...`);
    await apiClient.put('/api/power', { isPowerOn: newPowerState });

    // Update local state
    state.isPowerOn = newPowerState;
    updateUI();
    console.log(`Power turned ${state.isPowerOn ? 'on' : 'off'}`);
  } catch (error) {
    console.error('Error toggling power:', error);
    showError('Failed to toggle power state');
  }
}

// Toggle category playback
async function toggleCategoryPlayback(): Promise<void> {
  const newEnabledState = !state.categoryPlayback;

  try {
    // Update state on the server
    console.log(`Updating category playback state to ${newEnabledState}...`);
    await apiClient.put('/api/category-playback', { enabled: newEnabledState });

    // Update local state
    state.categoryPlayback = newEnabledState;

    if (newEnabledState) {
      // If enabling, resume last selected category only if it exists
      if (state.lastSelectedCategory) {
        await selectCategory(state.lastSelectedCategory);
      }
      startCategoryPlayback();
    } else {
      // If disabling, stop playback and play current.gif
      stopCategoryPlayback();

      // Call API to play default.gif
      try {
        await apiClient.post('/api/play-default', {});
        showSuccess('Playing default.gif');
      } catch (error) {
        console.error('Error playing default.gif:', error);
        showError('Failed to play default.gif');
      }
    }

    // Update UI and refresh category list
    updateUI();
    updateCategoryList();
    console.log(`Category ${newEnabledState ? 'enabled' : 'disabled'}`);
  } catch (error) {
    console.error('Error toggling category:', error);
    showError('Failed to toggle category');
  }
}

// Track the interval ID outside of state
let categoryPlayInterval: NodeJS.Timeout | null = null;

// Start category playback - now only handles the enabled state, no automatic cycling
function startCategoryPlayback(): void {
  // Clear any existing interval
  stopCategoryPlayback();
}

// Stop category playback
function stopCategoryPlayback(): void {
  if (categoryPlayInterval) {
    clearInterval(categoryPlayInterval);
    categoryPlayInterval = null;
  }
}

// Update category list in UI
function updateCategoryList(): void {
  if (!categoryList) return;

  categoryList.innerHTML = '';

  // Show loading state if data is loading
  if (state.isLoading) {
    const loadingItem = document.createElement('div');
    loadingItem.className = 'category-item loading';
    loadingItem.innerHTML = '<span class="category-name">Loading categories...</span>';
    categoryList.appendChild(loadingItem);
    return;
  }

  // Add categories if they exist
  if (state.categories.length > 0) {
    state.categories.forEach((category: string) => {
      const categoryItem = document.createElement('div');
      // Build class list with proper spacing
      let classList = 'category-item';
      if (state.categorySelected === category) {
        classList += ' active';
      }
      if (!state.categoryPlayback) {
        classList += ' disabled';
      }
      categoryItem.className = classList;
      categoryItem.innerHTML = `
        <span class="category-name">${category}</span>
        <div class="category-actions">
          <button class="action-btn play-btn material-icons" data-category-name="${category}">
            rocket_launch
          </button>
          <button class="action-btn edit-btn material-icons" data-category-name="${category}">
            edit
          </button>
          <button class="action-btn delete-btn material-icons" data-category-name="${category}">
            delete
          </button>
        </div>
      `;

      // Add click event for the whole category item
      const nameSpan = categoryItem.querySelector('.category-name') as HTMLElement;
      nameSpan.addEventListener('click', (e) => {
        e.stopPropagation();

        // Prevent category selection when categories are disabled
        if (!state.categoryPlayback) {
          showError('Category selection is disabled');
          return;
        }
  
        // Don't select the category when clicking on the text
        // Only the play button should select and play the category
        showError('Click the play button to select and play this category');
      });

      // Add click events for action buttons
      const playBtn = categoryItem.querySelector('.play-btn') as HTMLElement;
      playBtn.addEventListener('click', async (e) => {
        e.stopPropagation();

        // Prevent category selection when categories are disabled
        if (!state.categoryPlayback) {
          showError('Category selection is disabled');
          return;
        }

        // Select the category and play it
        await selectCategory(category);
        try {
          await apiClient.put('/api/category', { name: category });
          showSuccess(`Playing category: ${category}`);
        } catch (error) {
          console.error('Error setting category:', error);
          showError(`Failed to play category: ${error instanceof Error ? error.message : String(error)}`);
        }
      });

      const editBtn = categoryItem.querySelector('.edit-btn') as HTMLElement;
      editBtn.addEventListener('click', (e) => {
        e.stopPropagation();
        showRenameCategoryModal(category);
      });

      const deleteBtn = categoryItem.querySelector('.delete-btn') as HTMLElement;
      deleteBtn.addEventListener('click', (e) => {
        e.stopPropagation();
        showDeleteConfirmation(category);
      });

      categoryList.appendChild(categoryItem);
    });
  } else if (!state.isLoading) {
    // Show empty state if no categories
    const emptyItem = document.createElement('div');
    emptyItem.className = 'category-item empty';
    emptyItem.innerHTML = '<span class="category-name">No categories available</span>';
    categoryList.appendChild(emptyItem);
  }

  // Update category select in upload modal
  updateCategorySelect();
}

// Update category select dropdown in upload modal
function updateCategorySelect(): void {
  if (!categorySelect) return;

  categorySelect.innerHTML = '<option value="">Select a category...</option>';

  if (state.isLoading) {
    const option = document.createElement('option');
    option.value = '';
    option.textContent = 'Loading categories...';
    option.disabled = true;
    categorySelect.appendChild(option);
    return;
  }

  if (state.categories.length === 0) {
    const option = document.createElement('option');
    option.value = '';
    option.textContent = 'No categories available';
    option.disabled = true;
    categorySelect.appendChild(option);
    return;
  }

  state.categories.forEach((category: string) => {
    const option = document.createElement('option');
    option.value = category;
    option.textContent = category;
    categorySelect.appendChild(option);
  });
}

// Select a category
async function selectCategory(categoryName: string): Promise<void> {
  // Check if the category exists
  const categoryExists = state.categories.includes(categoryName);
  if (!categoryExists) {
    console.warn(`Category ${categoryName} not found`);
    // If no category is selected and we have categories, select the first one
    if (state.categories.length > 0) {
      state.categorySelected = state.categories[0];
    } else {
      state.categorySelected = '';
      return;
    }
  } else {
    state.categorySelected = categoryName;
  }

  // Store the last selected category
  state.lastSelectedCategory = state.categorySelected;

  // Update last selected category on the server
  try {
    await apiClient.put('/api/last-selected-category', { category: state.categorySelected });
  } catch (error) {
    console.error('Error updating last selected category:', error);
    showError(`Failed to update last selected category: ${error instanceof Error ? error.message : String(error)}`);
  }

  try {
    // Show loading state
    state.isLoading = true;

    // Category selected, no need to fetch GIFs anymore
  } catch (error) {
    console.error('Error fetching category:', error);
    showError(`Failed to load category: ${error instanceof Error ? error.message : String(error)}`);
    // No action needed
  } finally {
    state.isLoading = false;
    updateCategoryList();
  }
}

// Show rename category modal
function showRenameCategoryModal(category: string): void {
  if (renameCategoryInput && renameCategoryModal) {
    renameCategoryInput.value = category;
    renameCategoryInput.dataset.categoryName = category; // Store the original name in the input
    renameCategoryModal.style.display = 'block';
  }
}

// Rename a category
async function renameCategory(): Promise<void> {
  if (!renameCategoryInput || !renameCategoryModal) return;

  const originalName = renameCategoryInput.dataset.categoryName;
  const newName = renameCategoryInput.value.trim();

  if (!originalName || !newName) {
    showError('Please enter a category name');
    return;
  }

  try {
    // Show loading indicator
    if (loadingElement) loadingElement.style.display = 'block';

    // Call API to rename category
    await apiClient.put(`/api/categories/${originalName}`, { name: newName });

    // Update state with the renamed category
    const categoryIndex = state.categories.findIndex(c => c === originalName);
    if (categoryIndex !== -1) {
      state.categories[categoryIndex] = newName;
    }

    // Update UI
    updateCategoryList();
    if (renameCategoryModal) renameCategoryModal.style.display = 'none';

    // Show success message
    showSuccess(`Category renamed to "${newName}" successfully!`);
  } catch (error) {
    console.error('Error renaming category:', error);
    showError(error instanceof Error ? error.message : 'Failed to rename category');
  } finally {
    // Hide loading indicator
    if (loadingElement) loadingElement.style.display = 'none';
  }
}

// Show delete confirmation modal
function showDeleteConfirmation(category: string): void {
  console.log('Showing delete confirmation for:', category);
  if (deleteConfirmationText && deleteConfirmationModal) {
    deleteConfirmationText.textContent = `Are you sure you want to delete "${category}"?`;
    deleteConfirmationText.dataset.categoryName = category; // Store the category name in the text element
    console.log('Setting dataset.categoryName to:', category);
    deleteConfirmationModal.style.display = 'block';
  }
}

// Delete a category
async function deleteCategory(): Promise<void> {
  console.log('deleteCategory function called');
  if (!deleteConfirmationText || !deleteConfirmationModal) {
    console.log('Missing elements - deleteConfirmationText:', deleteConfirmationText, 'deleteConfirmationModal:', deleteConfirmationModal);
    return;
  }

  const categoryName = deleteConfirmationText.dataset.categoryName;
  console.log('Retrieved categoryName from dataset:', categoryName);

  if (!categoryName) {
    console.log('No categoryName found in dataset');
    return;
  }

  try {
    // Show loading indicator
    if (loadingElement) loadingElement.style.display = 'block';

    // Call API to delete category
    console.log('Deleting category:', categoryName);
    await apiClient.delete(`/api/categories/${categoryName}`);

    // Remove the category from state
    state.categories = state.categories.filter(c => c !== categoryName);

    // If the deleted category was selected, select the first available category
    if (state.categorySelected === categoryName) {
      if (state.categories.length > 0) {
        state.categorySelected = state.categories[0];
      } else {
        state.categorySelected = '';
      }
    }

    updateCategoryList();
    deleteConfirmationModal.style.display = 'none';

    // Show success message
    showSuccess(`Category "${categoryName}" deleted successfully!`);
  } catch (error) {
    console.error('Error deleting category:', error);
    showError(error instanceof Error ? error.message : 'Failed to delete category');
  } finally {
    // Hide loading indicator
    if (loadingElement) loadingElement.style.display = 'none';
  }
}

// Add a new category
async function addCategory(): Promise<void> {
  if (!newCategoryInput || !addCategoryModal) return;

  const categoryName = newCategoryInput.value.trim();
  if (!categoryName) {
    showError('Please enter a category name');
    return;
  }

  // Clear any previous error
  const addCategoryError = document.getElementById('add-category-error');
  if (addCategoryError) {
    addCategoryError.textContent = '';
    addCategoryError.style.display = 'none';
  }

  try {
    // Show loading indicator
    if (loadingElement) loadingElement.style.display = 'block';

    // Call API to add category
    console.log(`Adding new category: ${categoryName}`);
    await apiClient.post('/api/categories', { name: categoryName });

    // Reload categories from the backend to ensure we have the latest state
    await loadDataFromAPI();

    // Update UI
    updateCategoryList();
    addCategoryModal.style.display = 'none';
    newCategoryInput.value = '';

    // Show success message
    showSuccess(`Category "${categoryName}" added successfully!`);
  } catch (error) {
    console.error('Error adding category:', error);
    // Extract error message from the error object
    let errorMessage = 'Failed to add category';
    if (error instanceof Error) {
      errorMessage = error.message;
    }

    // Show error in the popup
    const addCategoryError = document.getElementById('add-category-error');
    if (addCategoryError) {
      addCategoryError.textContent = errorMessage;
      addCategoryError.style.display = 'block';
      addCategoryError.style.color = 'red';
    }
  } finally {
    // Hide loading indicator
    if (loadingElement) loadingElement.style.display = 'none';
  }
}

// Show error message
function showError(message: string): void {
  if (errorMessage && successMessage) {
    errorMessage.textContent = message;
    errorMessage.style.display = 'block';
    successMessage.style.display = 'none';
  }
}

// Show success message
function showSuccess(message: string): void {
  if (successMessage && errorMessage) {
    successMessage.textContent = message;
    successMessage.style.display = 'block';
    errorMessage.style.display = 'none';
  }
}

// Debounce function
/**
 * Prepare an image for upload - resizes all images to 64x64
 * @param file - The image file to prepare
 * @returns Promise that resolves with a File object ready for upload
 */
async function prepareImageForUpload(file: File): Promise<File> {
  // For GIFs, we need to resize them in the frontend
  if (file.type === 'image/gif') {
    console.log('GIF detected - resizing in frontend to 64x64');
    return resizeGifTo64x64(file);
  } else {
    // For non-GIF images, use the canvas approach to resize to 64x64
    return new Promise((resolve, reject) => {
      const img = new Image();
      const canvas = document.createElement('canvas');
      const ctx = canvas.getContext('2d');

      if (!ctx) {
        reject(new Error('Could not get canvas context'));
        return;
      }

      // Set canvas dimensions to 64x64
      canvas.width = 64;
      canvas.height = 64;

      const reader = new FileReader();
      reader.onload = (e) => {
        img.onload = () => {
          try {
            // Draw the image to the canvas with the new dimensions
            // Maintain aspect ratio
            let sourceWidth = img.width;
            let sourceHeight = img.height;
            let drawWidth = 64;
            let drawHeight = 64;

            // Calculate aspect ratio
            const aspectRatio = sourceWidth / sourceHeight;

            if (aspectRatio > 1) {
              // Landscape image
              drawHeight = 64 / aspectRatio;
            } else {
              // Portrait or square image
              drawWidth = 64 * aspectRatio;
            }

            // Center the image in the 64x64 canvas
            const xOffset = (64 - drawWidth) / 2;
            const yOffset = (64 - drawHeight) / 2;

            // Clear the canvas
            ctx.fillStyle = 'transparent';
            ctx.fillRect(0, 0, 64, 64);

            // Draw the image
            ctx.drawImage(img, xOffset, yOffset, drawWidth, drawHeight);

            // Convert canvas to a Blob
            canvas.toBlob((blob) => {
              if (blob) {
                // Create a new file with the resized image
                const fileName = file.name.replace(/\.[^/.]+$/, "") + ".gif";
                const resizedFile = new File([blob], fileName, {
                  type: 'image/gif',
                  lastModified: Date.now()
                });
                resolve(resizedFile);
              } else {
                reject(new Error('Failed to create blob from canvas'));
              }
            }, 'image/png');
          } catch (error) {
            reject(error);
          }
        };

        img.onerror = () => reject(new Error('Failed to load image'));
        img.src = e.target?.result as string;
      };

      reader.onerror = () => reject(new Error('Failed to read file'));
      reader.readAsDataURL(file);
    });
  }
}

/**
 * Resize a GIF to 64x64
 * @param file - The GIF file to resize
 * @returns Promise that resolves with a resized GIF file
 * Note: This creates a static GIF since we can't preserve animation in the frontend
 */
async function resizeGifTo64x64(file: File): Promise<File> {
  // For GIFs, we'll use a simpler approach since we can't use GIF.js
  // We'll just resize the first frame and create a static GIF
  // This is a limitation of the frontend-only approach

  const canvas = document.createElement('canvas');
  const ctx = canvas.getContext('2d');

  if (!ctx) {
    throw new Error('Could not get canvas context');
  }

  // Set canvas dimensions to 64x64
  canvas.width = 64;
  canvas.height = 64;

  const img = new Image();

  return new Promise((resolve, reject) => {
    const reader = new FileReader();
    reader.onload = (e) => {
      img.onload = () => {
        try {
          // Clear the canvas
          ctx.fillStyle = 'transparent';
          ctx.fillRect(0, 0, 64, 64);

          // Draw the image to the canvas with the new dimensions
          // Maintain aspect ratio
          let sourceWidth = img.width;
          let sourceHeight = img.height;
          let drawWidth = 64;
          let drawHeight = 64;

          // Calculate aspect ratio
          const aspectRatio = sourceWidth / sourceHeight;

          if (aspectRatio > 1) {
            // Landscape image
            drawHeight = 64 / aspectRatio;
          } else {
            // Portrait or square image
            drawWidth = 64 * aspectRatio;
          }

          // Center the image in the 64x64 canvas
          const xOffset = (64 - drawWidth) / 2;
          const yOffset = (64 - drawHeight) / 2;

          // Draw the image
          ctx.drawImage(img, xOffset, yOffset, drawWidth, drawHeight);

          // Convert canvas to a Blob
          canvas.toBlob((blob) => {
            if (blob) {
              // Create a new file with the resized image
              const fileName = file.name.replace(/\.[^/.]+$/, "") + ".gif";
              const resizedFile = new File([blob], fileName, {
                type: 'image/gif',
                lastModified: Date.now()
              });
              resolve(resizedFile);
            } else {
              reject(new Error('Failed to create blob from canvas'));
            }
          }, 'image/gif');
        } catch (error) {
          reject(error);
        }
      };

      img.onerror = () => reject(new Error('Failed to load image'));
      img.src = e.target?.result as string;
    };

    reader.onerror = () => reject(new Error('Failed to read file'));
    reader.readAsDataURL(file);
  });
}
function debounce(func: Function, wait: number): () => void {
  let timeout: NodeJS.Timeout;
  return function() {
    const context = this;
    const args = arguments;
    clearTimeout(timeout);
    timeout = setTimeout(() => {
      func.apply(context, args);
    }, wait);
  };
}

// Handle upload from the upload modal
async function handleUpload(): Promise<void> {
  console.log('handleUpload function called');

  // Get elements directly from the DOM
  const fileUpload = document.getElementById('file-upload') as HTMLInputElement | null;
  const categorySelect = document.getElementById('category-select') as HTMLSelectElement | null;
  const uploadModal = document.getElementById('upload-modal') as HTMLElement | null;

  if (!fileUpload || !categorySelect) {
    console.error('Missing required elements: fileUpload or categorySelect');
    showError('Internal error: Required elements not found');
    return;
  }

  const file = fileUpload.files?.[0];

  if (!file) {
    console.log('No file selected');
    showError('Please select a file');
    return;
  }

  console.log('File selected for upload:', file.name);
  console.log('Uploading to category:', categorySelect.value || 'default.gif');

  try {
    // Show loading indicator
    if (loadingElement) loadingElement.style.display = 'block';

    // Prepare the image for upload
    console.log('Preparing image for upload...');
    const preparedFile = await prepareImageForUpload(file);

    // Prepare form data for file upload
    const formData = new FormData();
    formData.append('file', preparedFile);

    // Add category if specified
    if (categorySelect.value) {
      formData.append('category', categorySelect.value);
      console.log('Category added to form data:', categorySelect.value);
    } else {
      console.log('No category selected, will upload as default.gif');
    }

    // Call API to upload image
    console.log('Uploading converted image via API...');
    const response = await apiClient.uploadFile('/api/gifs/upload', formData);
    console.log('Upload response:', response);

    // Show appropriate success message based on where the image was uploaded
    const successMessage = categorySelect.value
      ? `Image uploaded to category "${categorySelect.value}" successfully!`
      : 'Image uploaded as default.gif successfully!';

    showSuccess(successMessage);

    // Reset form
    fileUpload.value = '';
    uploadModal && (uploadModal.style.display = 'none');
  } catch (error) {
    console.error('Error uploading image:', error);
    showError(error instanceof Error ? error.message : 'Failed to upload image');
  } finally {
    // Hide loading indicator
    if (loadingElement) loadingElement.style.display = 'none';
  }
}

// Handle upload from the add category modal
async function handleUploadFromCategoryModal(): Promise<void> {
  if (!fileUploadCategory || !newCategoryInput) return;

  const file = fileUploadCategory.files?.[0];

  if (!file) {
    const uploadCategoryError = document.getElementById('upload-category-error');
    if (uploadCategoryError) {
      uploadCategoryError.textContent = 'Please select a file';
      uploadCategoryError.style.display = 'block';
    }
    return;
  }

  try {
    // Show loading indicator
    if (loadingElement) loadingElement.style.display = 'block';

    // Prepare the image for upload
    const preparedFile = await prepareImageForUpload(file);

    // Prepare form data for file upload
    const formData = new FormData();
    formData.append('file', preparedFile);

    // If a category name is provided, add it to the form data
    const categoryName = newCategoryInput.value.trim();
    if (categoryName) {
      formData.append('category', categoryName);
    }

    // Call API to upload image
    console.log('Uploading image via API from category modal...');
    await apiClient.uploadFile('/api/gifs/upload', formData);

    // Show appropriate success message based on where the image was uploaded
    const successMessage = categoryName
      ? `Image uploaded to category "${categoryName}" successfully!`
      : 'Image uploaded as default.gif successfully!';

    showSuccess(successMessage);

    // Reset form
    if (fileUploadCategory) fileUploadCategory.value = '';
  } catch (error) {
    console.error('Error uploading image:', error);
    const uploadCategoryError = document.getElementById('upload-category-error');
    if (uploadCategoryError) {
      uploadCategoryError.textContent = error instanceof Error ? error.message : 'Failed to upload image';
      uploadCategoryError.style.display = 'block';
    }
  } finally {
    // Hide loading indicator
    if (loadingElement) loadingElement.style.display = 'none';
  }
}

// Initialize the app
init();

