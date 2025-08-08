const fullscreenBtn = document.getElementById('fullscreen-btn');
const settingsBtn = document.getElementById('settings-btn');
const settingsPanel = document.getElementById('settings-panel');
const closeBtn = document.getElementById('close-settings-btn');
const saveBtn = document.getElementById('save-settings-btn');
const resetBtn = document.getElementById('reset-settings-btn');
const form = document.getElementById('settings-form');
const decreaseSpeedBtn = document.getElementById('decrease-speed-btn');
const increaseSpeedBtn = document.getElementById('increase-speed-btn');
const simulationSpeedEl = document.getElementById('simulation-speed');
const colorStopsContainer = document.getElementById('color-stops');
const addColorStopBtn = document.getElementById('add-color-stop-btn');
let wasmReady = false;

const defaultColorStops = [
  { color: '#9933ff', weight: 0.0 },
  { color: '#337fff', weight: 0.5 },
  { color: '#ffe533', weight: 0.75 },
  { color: '#ff4c33', weight: 1.0 },
];

let colorStops = [...defaultColorStops];

function renderColorStops() {
  colorStopsContainer.innerHTML = '';
  colorStops.forEach((stop, index) => {
    const stopEl = document.createElement('div');
    stopEl.classList.add('color-stop');
    stopEl.innerHTML = `
            <input type="color" value="${stop.color}" data-index="${index}">
            <input type="number" value="${stop.weight}" min="0" max="1" step="0.01" data-index="${index}">
            <button type="button" class="remove-color-stop-btn" data-index="${index}">X</button>
        `;
    colorStopsContainer.appendChild(stopEl);
  });

  const colorInputs = colorStopsContainer.querySelectorAll(
    'input[type="color"]'
  );
  const weightInputs = colorStopsContainer.querySelectorAll(
    'input[type="number"]'
  );
  const removeButtons = colorStopsContainer.querySelectorAll(
    '.remove-color-stop-btn'
  );

  colorInputs.forEach((input) =>
    input.addEventListener('change', (e) => {
      colorStops[e.target.dataset.index].color = e.target.value;
      applyColors();
    })
  );
  weightInputs.forEach((input) =>
    input.addEventListener('change', (e) => {
      colorStops[e.target.dataset.index].weight = parseFloat(e.target.value);
      applyColors();
    })
  );
  removeButtons.forEach((button) =>
    button.addEventListener('click', (e) => {
      colorStops.splice(e.target.dataset.index, 1);
      renderColorStops();
      applyColors();
    })
  );
}

addColorStopBtn.addEventListener('click', () => {
  colorStops.push({ color: '#ffffff', weight: 1.0 });
  renderColorStops();
  applyColors();
});

function getInitialColors() {
  const sortedStops = [...colorStops].sort((a, b) => a.weight - b.weight);
  const colors = [];
  const weights = [];
  sortedStops.forEach((stop) => {
    const r = parseInt(stop.color.slice(1, 3), 16) / 255;
    const g = parseInt(stop.color.slice(3, 5), 16) / 255;
    const b = parseInt(stop.color.slice(5, 7), 16) / 255;
    colors.push(r, g, b);
    weights.push(stop.weight);
  });
  return { colors, weights };
}

function applyColors() {
  if (!wasmReady) return;
  const { colors, weights } = getInitialColors();
  Module.setColors(colors, weights);
}

function updateSimulationSpeed() {
  if (!wasmReady) return;
  simulationSpeedEl.textContent = Module.getSimulationSpeed();
}

decreaseSpeedBtn.addEventListener('click', () => {
  if (!wasmReady) return;
  Module.decreaseSimulationSpeed();
  updateSimulationSpeed();
});

increaseSpeedBtn.addEventListener('click', () => {
  if (!wasmReady) return;
  Module.increaseSimulationSpeed();
  updateSimulationSpeed();
});

const simulationParameterKeys = [
  'G',
  'DENSITY',
  'NUM_BODIES',
  'INITIALIZATION_RADIUS',
  'DT',
  'SOFTENING_FACTOR',
  'MAX_MASS',
  'MIN_MASS',
  'CENTRAL_BODY_MASS',
  'THETA',
];

function populateSettingsForm() {
  if (!wasmReady) return;
  updateSimulationSpeed();
  const params = Module.getSimulationParameters();
  for (const key of simulationParameterKeys) {
    if (form.elements[key] && params.hasOwnProperty(key)) {
      const value = params[key];
      if (form.elements[key].step === 'any') {
        form.elements[key].value = parseFloat(value.toPrecision(7));
      } else {
        form.elements[key].value = value;
      }
    }
  }
}

function saveSettings() {
  const settings = {};
  for (const key of simulationParameterKeys) {
    if (form.elements[key]) {
      settings[key] = form.elements[key].value;
    }
  }
  settings.colorStops = colorStops;
  localStorage.setItem('simulationSettings', JSON.stringify(settings));
}

function loadSettings() {
  const savedSettings = localStorage.getItem('simulationSettings');
  if (savedSettings) {
    const settings = JSON.parse(savedSettings);
    if (settings.colorStops) {
      colorStops = settings.colorStops;
    }
    const newParams = {};
    for (const key of simulationParameterKeys) {
      if (settings.hasOwnProperty(key)) {
        newParams[key] = Number(settings[key]) || 0;
      }
    }
    Module.setSimulationParameters(newParams);
  }
}

function applySettings() {
  if (!wasmReady) return;
  const newParams = {};
  for (const key of simulationParameterKeys) {
    if (form.elements[key]) {
      newParams[key] = Number(form.elements[key].value) || 0;
    }
  }
  Module.setSimulationParameters(newParams);
  applyColors();
  saveSettings();
  settingsPanel.classList.add('hidden');
  console.log('Settings applied and saved.');
}

function resetSettings() {
  console.log('Resetting settings to default by reloading.');
  localStorage.removeItem('simulationSettings');
  window.location.reload();
}

settingsBtn.addEventListener('click', () => {
  populateSettingsForm();
  renderColorStops();
  settingsPanel.classList.toggle('hidden');
});

closeBtn.addEventListener('click', () => {
  settingsPanel.classList.add('hidden');
});

saveBtn.addEventListener('click', applySettings);
resetBtn.addEventListener('click', resetSettings);

fullscreenBtn.addEventListener('click', () => {
  if (!document.fullscreenElement) {
    document.documentElement.requestFullscreen();
  } else {
    if (document.exitFullscreen) {
      document.exitFullscreen();
    }
  }
});

var Module = {
  canvas: (function () {
    return document.getElementById('canvas');
  })(),
  onRuntimeInitialized: function () {
    console.log('WASM Runtime Initialized.');
    wasmReady = true;
    saveBtn.disabled = false;

    loadSettings();

    populateSettingsForm();
    renderColorStops();
    applyColors();
  },
};

saveBtn.disabled = true;
