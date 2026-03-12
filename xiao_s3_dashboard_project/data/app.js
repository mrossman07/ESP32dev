let ws;

async function fetchJson(url, options = {}) {
  const r = await fetch(url, options);
  if (!r.ok) throw new Error(`${r.status} ${r.statusText}`);
  return await r.json();
}

function fmtUptime(ms) {
  const s = Math.floor(ms / 1000);
  const h = Math.floor(s / 3600);
  const m = Math.floor((s % 3600) / 60);
  const sec = s % 60;
  return `${h}h ${m}m ${sec}s`;
}

function updateStatus(s) {
  document.getElementById('deviceName').textContent = s.deviceName ?? '-';
  document.getElementById('wifiConnected').textContent = s.wifiConnected ? 'yes' : 'no';
  document.getElementById('ssid').textContent = s.ssid || '-';
  document.getElementById('ip').textContent = s.ip || '-';
  document.getElementById('mdns').textContent = s.mdns || '-';
  document.getElementById('rssi').textContent = s.wifiConnected ? `${s.rssi} dBm` : '-';
  document.getElementById('heap').textContent = `${s.freeHeap} bytes`;
  document.getElementById('uptime').textContent = fmtUptime(s.uptimeMs);
  document.getElementById('analogRaw').textContent = s.analogRaw;
  document.getElementById('analogVolts').textContent = Number(s.analogVolts).toFixed(3) + ' V';
  document.getElementById('sampleCount').textContent = s.sampleCount;
  document.getElementById('logSizeBytes').textContent = `${s.logSizeBytes} bytes`;
}

async function loadStatusOnce() {
  updateStatus(await fetchJson('/api/status'));
}

async function loadConfig() {
  const c = await fetchJson('/api/config');
  document.getElementById('cfgDeviceName').value = c.deviceName || '';
  document.getElementById('cfgSsid').value = c.wifiSsid || '';
  document.getElementById('cfgAnalogPin').value = c.analogPin ?? 1;
  document.getElementById('cfgSampleMs').value = c.sampleIntervalMs ?? 1000;
}

async function saveConfig() {
  const body = {
    deviceName: document.getElementById('cfgDeviceName').value.trim(),
    wifiSsid: document.getElementById('cfgSsid').value.trim(),
    wifiPassword: document.getElementById('cfgPassword').value,
    newAdminPassword: document.getElementById('cfgNewAdminPassword').value,
    analogPin: Number(document.getElementById('cfgAnalogPin').value),
    sampleIntervalMs: Number(document.getElementById('cfgSampleMs').value),
    password: document.getElementById('cfgAdminPassword').value
  };

  const r = await fetchJson('/api/config', {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify(body)
  });

  document.getElementById('configResult').textContent = JSON.stringify(r, null, 2);
  await loadStats();
  await loadSensors();
}

async function restartDevice() {
  const body = { password: document.getElementById('cfgAdminPassword').value };
  const r = await fetchJson('/api/restart', {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify(body)
  });
  document.getElementById('configResult').textContent = JSON.stringify(r, null, 2);
}

async function loadStats() {
  const s = await fetchJson('/api/stats');
  document.getElementById('statsBox').textContent = JSON.stringify(s, null, 2);
}

async function loadSensors() {
  const s = await fetchJson('/api/sensors');
  document.getElementById('sensorBox').textContent = JSON.stringify(s, null, 2);
}

async function loadLogs() {
  const r = await fetch('/logs.txt');
  document.getElementById('logsBox').textContent = await r.text();
}

function connectWs() {
  const host = window.location.hostname;
  ws = new WebSocket(`ws://${host}:81/`);
  const state = document.getElementById('wsState');

  ws.onopen = () => {
    state.textContent = 'WS: connected';
    ws.send('status');
  };

  ws.onmessage = (ev) => {
    try {
      updateStatus(JSON.parse(ev.data));
    } catch (e) {
      console.error(e);
    }
  };

  ws.onclose = () => {
    state.textContent = 'WS: disconnected';
    setTimeout(connectWs, 2000);
  };

  ws.onerror = () => {
    state.textContent = 'WS: error';
  };
}

document.getElementById('refreshBtn').addEventListener('click', loadStatusOnce);
document.getElementById('saveBtn').addEventListener('click', saveConfig);
document.getElementById('restartBtn').addEventListener('click', restartDevice);
document.getElementById('loadLogsBtn').addEventListener('click', loadLogs);

(async function init() {
  try {
    await loadConfig();
    await loadStatusOnce();
    await loadStats();
    await loadSensors();
    connectWs();
  } catch (e) {
    document.getElementById('configResult').textContent = String(e);
  }
})();
