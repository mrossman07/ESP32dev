/*
 * ESP32 WebSocket Live Dashboard Server
 * ------------------------------------
 * Libraries required (install via Arduino Library Manager):
 *   - ESPAsyncWebServer  (by me-no-dev)
 *   - AsyncTCP           (by me-no-dev)
 * Make sure you get the mathieucarbou versions
 * /github.com/mathieucarbou/..
 *
 * Board: ESP32 Dev Module (or any ESP32 variant)
 */

#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>

// ── Wi-Fi credentials ───────────────────────────────────────────────────────
const char* WIFI_SSID     = "gatornet";
const char* WIFI_PASSWORD = "g8orWAP2025!";

// ── Server & WebSocket ──────────────────────────────────────────────────────
AsyncWebServer server(80);
AsyncWebSocket  ws("/ws");

// ── Simulated sensor state ──────────────────────────────────────────────────
float    temperature  = 24.0;
float    humidity     = 55.0;
int      lightLevel   = 512;
bool     ledState     = false;
uint32_t uptimeSeconds = 0;

// Broadcast interval (ms)
const uint32_t BROADCAST_INTERVAL = 1000;
uint32_t lastBroadcast = 0;

// ── Embedded HTML page ──────────────────────────────────────────────────────
// Stored in program flash (PROGMEM) to save RAM
const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>ESP32 Live Dashboard</title>
<style>
  @import url('https://fonts.googleapis.com/css2?family=Share+Tech+Mono&family=Exo+2:wght@300;600;800&display=swap');

  :root {
    --bg:       #0a0e1a;
    --panel:    #111827;
    --border:   #1e3a5f;
    --accent:   #00d4ff;
    --accent2:  #ff6b35;
    --green:    #00ff88;
    --text:     #c8d6e5;
    --dim:      #4a6080;
    --glow:     0 0 18px rgba(0,212,255,0.35);
  }

  *, *::before, *::after { box-sizing: border-box; margin: 0; padding: 0; }

  body {
    background: var(--bg);
    color: var(--text);
    font-family: 'Exo 2', sans-serif;
    min-height: 100vh;
    padding: 24px 16px 40px;
    background-image:
      radial-gradient(ellipse 80% 50% at 50% -20%, rgba(0,100,180,0.18) 0%, transparent 60%),
      repeating-linear-gradient(0deg, transparent, transparent 39px, rgba(0,212,255,0.03) 40px),
      repeating-linear-gradient(90deg, transparent, transparent 39px, rgba(0,212,255,0.03) 40px);
  }

  header {
    text-align: center;
    margin-bottom: 32px;
  }
  header h1 {
    font-size: clamp(1.6rem, 4vw, 2.4rem);
    font-weight: 800;
    letter-spacing: 0.08em;
    color: #fff;
    text-transform: uppercase;
  }
  header h1 span { color: var(--accent); }

  .status-bar {
    display: flex;
    align-items: center;
    justify-content: center;
    gap: 10px;
    margin-top: 8px;
    font-family: 'Share Tech Mono', monospace;
    font-size: 0.78rem;
    color: var(--dim);
  }
  .dot {
    width: 8px; height: 8px;
    border-radius: 50%;
    background: var(--dim);
    transition: background 0.4s, box-shadow 0.4s;
  }
  .dot.live { background: var(--green); box-shadow: 0 0 8px var(--green); }

  /* ── Grid ── */
  .grid {
    display: grid;
    gap: 16px;
    max-width: 800px;
    margin: 0 auto;
    grid-template-columns: repeat(auto-fit, minmax(200px, 1fr));
  }

  /* ── Cards ── */
  .card {
    background: var(--panel);
    border: 1px solid var(--border);
    border-radius: 12px;
    padding: 22px 20px 18px;
    position: relative;
    overflow: hidden;
    transition: border-color 0.3s;
  }
  .card::before {
    content: '';
    position: absolute;
    inset: 0;
    background: linear-gradient(135deg, rgba(0,212,255,0.04) 0%, transparent 60%);
    pointer-events: none;
  }
  .card:hover { border-color: var(--accent); box-shadow: var(--glow); }

  .card-icon { font-size: 1.6rem; margin-bottom: 6px; }
  .card-label {
    font-size: 0.68rem;
    font-weight: 600;
    letter-spacing: 0.14em;
    text-transform: uppercase;
    color: var(--dim);
    margin-bottom: 6px;
  }
  .card-value {
    font-family: 'Share Tech Mono', monospace;
    font-size: clamp(1.8rem, 4vw, 2.4rem);
    color: #fff;
    line-height: 1;
  }
  .card-value span { font-size: 0.9rem; color: var(--dim); }

  /* colour accents per card */
  .card.temp  .card-value { color: var(--accent2); }
  .card.humid .card-value { color: var(--accent);  }
  .card.light .card-value { color: #f9c74f; }
  .card.up    .card-value { color: var(--green); font-size: 1.3rem; }

  /* progress bar */
  .bar-wrap {
    margin-top: 10px;
    height: 4px;
    background: rgba(255,255,255,0.07);
    border-radius: 4px;
    overflow: hidden;
  }
  .bar {
    height: 100%;
    border-radius: 4px;
    transition: width 0.8s ease;
  }
  .card.temp  .bar { background: linear-gradient(90deg, #ff6b35, #ff9a00); }
  .card.humid .bar { background: linear-gradient(90deg, #00aaff, #00d4ff); }
  .card.light .bar { background: linear-gradient(90deg, #f9c74f, #ffdd8a); }

  /* ── LED toggle ── */
  .card.led { grid-column: span 2; }
  @media (max-width: 480px) { .card.led { grid-column: span 1; } }

  .led-row {
    display: flex;
    align-items: center;
    justify-content: space-between;
    flex-wrap: wrap;
    gap: 12px;
  }
  .led-indicator {
    width: 28px; height: 28px;
    border-radius: 50%;
    background: #1a2a1a;
    border: 2px solid #2a4a2a;
    transition: background 0.3s, box-shadow 0.3s, border-color 0.3s;
  }
  .led-indicator.on {
    background: var(--green);
    border-color: var(--green);
    box-shadow: 0 0 16px var(--green), 0 0 32px rgba(0,255,136,0.4);
  }

  .btn {
    font-family: 'Exo 2', sans-serif;
    font-weight: 600;
    font-size: 0.82rem;
    letter-spacing: 0.1em;
    text-transform: uppercase;
    padding: 10px 24px;
    border: 1px solid var(--accent);
    border-radius: 8px;
    background: transparent;
    color: var(--accent);
    cursor: pointer;
    transition: background 0.2s, color 0.2s, box-shadow 0.2s;
  }
  .btn:hover {
    background: var(--accent);
    color: var(--bg);
    box-shadow: var(--glow);
  }
  .btn:active { transform: scale(0.97); }

  /* ── Log ── */
  .log-wrap {
    max-width: 800px;
    margin: 20px auto 0;
  }
  .log-label {
    font-size: 0.68rem;
    font-weight: 600;
    letter-spacing: 0.14em;
    text-transform: uppercase;
    color: var(--dim);
    margin-bottom: 6px;
  }
  #log {
    background: var(--panel);
    border: 1px solid var(--border);
    border-radius: 10px;
    padding: 12px 14px;
    height: 110px;
    overflow-y: auto;
    font-family: 'Share Tech Mono', monospace;
    font-size: 0.72rem;
    color: var(--dim);
    line-height: 1.7;
  }
  #log .entry { color: var(--text); }
  #log .entry.cmd { color: var(--accent2); }
</style>
</head>
<body>

<header>
  <h1>ESP32 <span>Live</span> Dashboard</h1>
  <div class="status-bar">
    <div class="dot" id="dot"></div>
    <span id="ws-status">Connecting…</span>
    &nbsp;|&nbsp;
    <span id="ip-label"></span>
  </div>
</header>

<div class="grid">

  <div class="card temp">
    <div class="card-icon">🌡️</div>
    <div class="card-label">Temperature</div>
    <div class="card-value" id="temp">--<span> °C</span></div>
    <div class="bar-wrap"><div class="bar" id="temp-bar" style="width:0%"></div></div>
  </div>

  <div class="card humid">
    <div class="card-icon">💧</div>
    <div class="card-label">Humidity</div>
    <div class="card-value" id="humid">--<span> %</span></div>
    <div class="bar-wrap"><div class="bar" id="humid-bar" style="width:0%"></div></div>
  </div>

  <div class="card light">
    <div class="card-icon">☀️</div>
    <div class="card-label">Light Level</div>
    <div class="card-value" id="light">--<span> /1023</span></div>
    <div class="bar-wrap"><div class="bar" id="light-bar" style="width:0%"></div></div>
  </div>

  <div class="card up">
    <div class="card-icon">⏱️</div>
    <div class="card-label">Uptime</div>
    <div class="card-value" id="uptime">--</div>
  </div>

  <div class="card led">
    <div class="card-label" style="margin-bottom:14px">Built-in LED Control</div>
    <div class="led-row">
      <div style="display:flex;align-items:center;gap:14px">
        <div class="led-indicator" id="led-dot"></div>
        <span id="led-status" style="font-family:'Share Tech Mono',monospace;font-size:0.9rem">OFF</span>
      </div>
      <button class="btn" id="toggle-btn" onclick="toggleLED()">Toggle LED</button>
    </div>
  </div>

</div>

<div class="log-wrap">
  <div class="log-label">WebSocket Log</div>
  <div id="log"></div>
</div>

<script>
  const dot     = document.getElementById('dot');
  const wsStat  = document.getElementById('ws-status');
  const ipLabel = document.getElementById('ip-label');
  const logEl   = document.getElementById('log');

  function logMsg(msg, cls = 'entry') {
    const d = new Date();
    const t = d.toTimeString().slice(0,8);
    logEl.innerHTML += `<div class="${cls}">[${t}] ${msg}</div>`;
    logEl.scrollTop = logEl.scrollHeight;
    if (logEl.children.length > 60) logEl.removeChild(logEl.firstChild);
  }

  function fmtUptime(s) {
    const h = Math.floor(s / 3600).toString().padStart(2,'0');
    const m = Math.floor((s % 3600) / 60).toString().padStart(2,'0');
    const sec = (s % 60).toString().padStart(2,'0');
    return `${h}:${m}:${sec}`;
  }

  // ── WebSocket ──────────────────────────────────────────────────────────────
  let ws, reconnectTimer;

  function connect() {
    const host = location.hostname;
    ipLabel.textContent = host;
    ws = new WebSocket(`ws://${host}/ws`);

    ws.onopen = () => {
      dot.classList.add('live');
      wsStat.textContent = 'Connected';
      logMsg('WebSocket connected ✓');
      clearTimeout(reconnectTimer);
    };

    ws.onclose = () => {
      dot.classList.remove('live');
      wsStat.textContent = 'Disconnected — retrying…';
      logMsg('Connection lost, reconnecting in 3 s…');
      reconnectTimer = setTimeout(connect, 3000);
    };

    ws.onerror = (e) => logMsg('WebSocket error');

    ws.onmessage = (evt) => {
      try {
        const d = JSON.parse(evt.data);

        if (d.type === 'state') {
          document.getElementById('temp').innerHTML   = `${d.temperature.toFixed(1)}<span> °C</span>`;
          document.getElementById('humid').innerHTML  = `${d.humidity.toFixed(1)}<span> %</span>`;
          document.getElementById('light').innerHTML  = `${d.light}<span> /1023</span>`;
          document.getElementById('uptime').textContent = fmtUptime(d.uptime);

          document.getElementById('temp-bar').style.width  = `${Math.min((d.temperature / 60) * 100, 100)}%`;
          document.getElementById('humid-bar').style.width = `${d.humidity}%`;
          document.getElementById('light-bar').style.width = `${(d.light / 1023) * 100}%`;

          const ledOn = d.led;
          document.getElementById('led-dot').classList.toggle('on', ledOn);
          document.getElementById('led-status').textContent = ledOn ? 'ON' : 'OFF';

          logMsg(`temp=${d.temperature.toFixed(1)}°C  hum=${d.humidity.toFixed(1)}%  light=${d.light}  led=${ledOn ? 'ON' : 'OFF'}`);
        }
      } catch(e) { logMsg('Parse error: ' + evt.data); }
    };
  }

  function toggleLED() {
    if (ws && ws.readyState === WebSocket.OPEN) {
      ws.send(JSON.stringify({ cmd: 'toggleLED' }));
      logMsg('→ sent: toggleLED', 'entry cmd');
    } else {
      logMsg('Not connected', 'entry cmd');
    }
  }

  connect();
</script>
</body>
</html>
)rawliteral";


// ── WebSocket event handler ─────────────────────────────────────────────────
void onWsEvent(AsyncWebSocket* server, AsyncWebSocketClient* client,
               AwsEventType type, void* arg, uint8_t* data, size_t len)
{
  if (type == WS_EVT_CONNECT) {
    Serial.printf("[WS] Client #%u connected from %s\n",
                  client->id(), client->remoteIP().toString().c_str());
    // Send current state immediately on connect
    broadcastState();

  } else if (type == WS_EVT_DISCONNECT) {
    Serial.printf("[WS] Client #%u disconnected\n", client->id());

  } else if (type == WS_EVT_DATA) {
    AwsFrameInfo* info = (AwsFrameInfo*)arg;
    if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
      // Null-terminate and parse
      String msg = String((char*)data).substring(0, len);
      Serial.printf("[WS] Message from #%u: %s\n", client->id(), msg.c_str());

      // Simple command parsing (no JSON library needed for a single key)
      if (msg.indexOf("toggleLED") != -1) {
        ledState = !ledState;
        digitalWrite(LED_BUILTIN, ledState ? LOW : HIGH);
        Serial.printf("[WS] LED toggled → %s\n", ledState ? "ON" : "OFF");
        broadcastState();   // push update to all clients immediately
      }
    }
  } else if (type == WS_EVT_ERROR) {
    Serial.printf("[WS] Error on client #%u\n", client->id());
  }
}

// ── Build & broadcast JSON state ────────────────────────────────────────────
void broadcastState() {
  // Build JSON manually (avoids ArduinoJson dependency)
  char buf[256];
  snprintf(buf, sizeof(buf),
    "{\"type\":\"state\","
    "\"temperature\":%.2f,"
    "\"humidity\":%.2f,"
    "\"light\":%d,"
    "\"led\":%s,"
    "\"uptime\":%lu}",
    temperature, humidity, lightLevel,
    ledState ? "true" : "false",
    uptimeSeconds);

  ws.textAll(buf);
}

// ── Simulate sensor readings ─────────────────────────────────────────────────
void updateSensors() {
  // Replace these with real sensor reads, e.g.:
  //   temperature = dht.readTemperature();
  //   humidity    = dht.readHumidity();
  //   lightLevel  = analogRead(34);

  temperature  += (float)(random(-20, 21)) / 10.0f;  // ±2 °C drift
  humidity     += (float)(random(-10, 11)) / 10.0f;  // ±1 % drift
  lightLevel   += random(-30, 31);

  // Clamp values
  temperature = constrain(temperature, 10.0f, 55.0f);
  humidity    = constrain(humidity,     0.0f, 100.0f);
  lightLevel  = constrain(lightLevel,   0,    1023);

  uptimeSeconds = millis() / 1000;
}

// ── setup ───────────────────────────────────────────────────────────────────
void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);

  // Connect to Wi-Fi
  Serial.printf("\nConnecting to %s", WIFI_SSID);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print('.'); }
  Serial.printf("\nConnected! IP: %s\n", WiFi.localIP().toString().c_str());

  // Attach WebSocket handler
  ws.onEvent(onWsEvent);
  server.addHandler(&ws);

  // Serve the HTML page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest* req) {
    req->send_P(200, "text/html", INDEX_HTML);
  });

  // 404 fallback
  server.onNotFound([](AsyncWebServerRequest* req) {
    req->send(404, "text/plain", "Not found");
  });

  server.begin();
  Serial.println("HTTP server started. Open: http://" + WiFi.localIP().toString());
}

// ── loop ────────────────────────────────────────────────────────────────────
void loop() {
  ws.cleanupClients();   // remove stale connections

  uint32_t now = millis();
  if (now - lastBroadcast >= BROADCAST_INTERVAL) {
    lastBroadcast = now;
    updateSensors();
    broadcastState();
  }
}
