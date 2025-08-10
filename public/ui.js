const fullscreenBtn = document.getElementById('fullscreen-btn');

function encodeSimulationData(data) {
  let buffer = new ArrayBuffer(1024); // Start with a 1KB buffer
  let view = new DataView(buffer);
  let offset = 0;

  function ensureCapacity(needed) {
    if (buffer.byteLength < offset + needed) {
      const newBuffer = new ArrayBuffer(buffer.byteLength * 2);
      new Uint8Array(newBuffer).set(new Uint8Array(buffer));
      buffer = newBuffer;
      view = new DataView(buffer);
    }
  }

  // Encode parameters
  for (const key of simulationParameterKeys) {
    ensureCapacity(4);
    view.setFloat32(offset, data.parameters[key], true);
    offset += 4;
  }

  // Encode bodies
  ensureCapacity(4);
  view.setUint32(offset, data.bodies.length, true);
  offset += 4;
  for (const body of data.bodies) {
    ensureCapacity(28); // 6 floats + 1 int per body
    view.setFloat32(offset, body.x, true);
    offset += 4;
    view.setFloat32(offset, body.y, true);
    offset += 4;
    view.setFloat32(offset, body.vx, true);
    offset += 4;
    view.setFloat32(offset, body.vy, true);
    offset += 4;
    view.setFloat32(offset, body.mass, true);
    offset += 4;
    view.setInt32(offset, body.id, true);
    offset += 4;
    view.setFloat32(offset, body.radius, true);
    offset += 4;
  }

  // Encode colors
  ensureCapacity(4);
  view.setUint32(offset, data.colors.length, true);
  offset += 4;
  for (const color of data.colors) {
    ensureCapacity(7); // 3 bytes for color, 4 for weight
    const r = parseInt(color.color.slice(1, 3), 16);
    const g = parseInt(color.color.slice(3, 5), 16);
    const b = parseInt(color.color.slice(5, 7), 16);
    view.setUint8(offset, r);
    offset += 1;
    view.setUint8(offset, g);
    offset += 1;
    view.setUint8(offset, b);
    offset += 1;
    view.setFloat32(offset, color.weight, true);
    offset += 4;
  }

  // Convert to base64
  const binary = new Uint8Array(buffer, 0, offset);
  const binaryString = Array.from(binary)
    .map((byte) => String.fromCharCode(byte))
    .join('');
  const base64 = btoa(binaryString);
  return base64.replace(/\+/g, '-').replace(/\//g, '_').replace(/=/g, '');
}

function decodeSimulationData(encodedData) {
  let base64 = encodedData.replace(/-/g, '+').replace(/_/g, '/');
  while (base64.length % 4) {
    base64 += '=';
  }
  const binaryString = atob(base64);
  const buffer = new ArrayBuffer(binaryString.length);
  const view = new DataView(buffer);
  for (let i = 0; i < binaryString.length; i++) {
    view.setUint8(i, binaryString.charCodeAt(i));
  }

  let offset = 0;
  const data = {
    parameters: {},
    bodies: [],
    colors: [],
  };

  // Decode parameters
  for (const key of simulationParameterKeys) {
    data.parameters[key] = view.getFloat32(offset, true);
    offset += 4;
  }

  // Decode bodies
  const numBodies = view.getUint32(offset, true);
  offset += 4;
  for (let i = 0; i < numBodies; i++) {
    const body = {};
    body.x = view.getFloat32(offset, true);
    offset += 4;
    body.y = view.getFloat32(offset, true);
    offset += 4;
    body.vx = view.getFloat32(offset, true);
    offset += 4;
    body.vy = view.getFloat32(offset, true);
    offset += 4;
    body.mass = view.getFloat32(offset, true);
    offset += 4;
    body.id = view.getInt32(offset, true);
    offset += 4;
    body.radius = view.getFloat32(offset, true);
    offset += 4;
    data.bodies.push(body);
  }

  // Decode colors
  const numColors = view.getUint32(offset, true);
  offset += 4;
  for (let i = 0; i < numColors; i++) {
    const color = {};
    const r = view.getUint8(offset);
    offset += 1;
    const g = view.getUint8(offset);
    offset += 1;
    const b = view.getUint8(offset);
    offset += 1;
    color.color = `#${r.toString(16).padStart(2, '0')}${g.toString(16).padStart(2, '0')}${b.toString(16).padStart(2, '0')}`;
    color.weight = view.getFloat32(offset, true);
    offset += 4;
    data.colors.push(color);
  }

  return data;
}
const settingsBtn = document.getElementById('settings-btn');
const shareBtn = document.getElementById('share-btn');
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
  console.log('Resetting settings to default.');
  localStorage.removeItem('simulationSettings');
  colorStops = [...defaultColorStops];
  if (wasmReady) {
    Module.resetSimulationToDefaults();
    populateSettingsForm();
    renderColorStops();
    applyColors();
  }
  settingsPanel.classList.add('hidden');
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

shareBtn.addEventListener('click', () => {
  if (!wasmReady) return;

  const MAX_URL_LENGTH = 2000;
  const baseUrl = `${window.location.origin}${window.location.pathname}`;
  const MAX_BASE64_LENGTH =
    MAX_URL_LENGTH - (baseUrl.length + '?simulation='.length);
  const MAX_BINARY_SIZE = Math.floor((MAX_BASE64_LENGTH * 3) / 4);

  const parameters = Module.getSimulationParameters();
  let bodies = Module.getBodies();
  const colors = colorStops;

  const paramSize = simulationParameterKeys.length * 4;
  const colorsSize = 4 + colors.length * 7;
  const fixedSize = paramSize + 4 /* num_bodies */ + colorsSize;

  const maxBodies = Math.floor((MAX_BINARY_SIZE - fixedSize) / 28);

  if (bodies.length > maxBodies) {
    alert(
      `Warning: The simulation contains too many bodies to share in a URL. Only the ${maxBodies} bodies closest to the center will be included in the sharable link.`
    );

    bodies.sort((a, b) => {
      const distA = Math.sqrt(a.x * a.x + a.y * a.y);
      const distB = Math.sqrt(b.x * b.x + b.y * b.y);
      return distA - distB;
    });

    bodies = bodies.slice(0, maxBodies);
  }

  const simulationData = {
    parameters: parameters,
    bodies: bodies,
    colors: colors,
  };

  const encodedData = encodeSimulationData(simulationData);
  const url = `${baseUrl}?simulation=${encodedData}`;

  navigator.clipboard.writeText(url).then(
    () => {
      alert('Sharable link copied to clipboard!');
    },
    () => {
      alert('Failed to copy sharable link.');
    }
  );
});

let wakeLock = null;

const requestWakeLock = async () => {
  try {
    wakeLock = await navigator.wakeLock.request('screen');
    wakeLock.addEventListener('release', () => {
      console.log('Screen Wake Lock released:', wakeLock.released);
    });
    console.log('Screen Wake Lock is active.');
  } catch (err) {
    console.error(`${err.name}, ${err.message}`);
  }
};

const releaseWakeLock = async () => {
  if (wakeLock !== null) {
    await wakeLock.release();
    wakeLock = null;
  }
};

fullscreenBtn.addEventListener('click', () => {
  if (!document.fullscreenElement) {
    document.documentElement.requestFullscreen();
  } else {
    if (document.exitFullscreen) {
      document.exitFullscreen();
    }
  }
});

document.addEventListener('fullscreenchange', async () => {
  if (document.fullscreenElement) {
    await requestWakeLock();
  } else {
    await releaseWakeLock();
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

    const urlParams = new URLSearchParams(window.location.search);
    const simulationData = urlParams.get('simulation');

    if (simulationData) {
      try {
        const parsedData = decodeSimulationData(simulationData);

        if (parsedData.parameters) {
          Module.setSimulationParameters(parsedData.parameters);
        }
        if (parsedData.bodies) {
          Module.setBodies(parsedData.bodies);
          Module.markStateAsLoaded();
        }
        if (parsedData.colors) {
          colorStops = parsedData.colors;
        }
      } catch (e) {
        console.error('Failed to load simulation from URL:', e);
        loadSettings();
      }
    } else {
      loadSettings();
    }

    populateSettingsForm();
    renderColorStops();
    applyColors();
  },
};

saveBtn.disabled = true;

let hideTimeout;

function showUI() {
  fullscreenBtn.classList.remove('hidden-ui');
  settingsBtn.classList.remove('hidden-ui');
  shareBtn.classList.remove('hidden-ui');
  clearTimeout(hideTimeout);
  hideTimeout = setTimeout(() => {
    fullscreenBtn.classList.add('hidden-ui');
    settingsBtn.classList.add('hidden-ui');
    shareBtn.classList.add('hidden-ui');
  }, 3000);
}

document.body.addEventListener('mousemove', showUI);
document.body.addEventListener('touchstart', showUI);

showUI();
