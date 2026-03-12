#include "AppServer.h"

static constexpr bool FORMAT_LITTLEFS_IF_FAILED = true;

void AppServer::begin() {
  Serial.begin(115200);
  delay(600);

  if (!LittleFS.begin(FORMAT_LITTLEFS_IF_FAILED)) {
    Serial.println("LittleFS mount failed");
    while (true) delay(1000);
  }

  logs.begin();
  configManager.begin();
  configManager.load(config);
  stats.begin();
  stats.incrementBootCount();
  sensors.begin(config.analogPin, config.sampleIntervalMs);

  logs.append("Boot");
  connectWifi();
  startMdns();
  registerRoutes();
  registerSocket();
  ota.begin(&logs, &stats, &config.adminPassword);
  ota.registerRoutes(server);

  server.begin();
  webSocket.begin();
  webSocket.onEvent([this](uint8_t num, WStype_t type, uint8_t* payload, size_t length) {
    switch (type) {
      case WStype_CONNECTED:
        logs.append("WebSocket client connected");
        webSocket.sendTXT(num, getStatusJson());
        break;
      case WStype_TEXT: {
        String msg((char*)payload, length);
        if (msg == "status") webSocket.sendTXT(num, getStatusJson());
        break;
      }
      case WStype_DISCONNECTED:
        logs.append("WebSocket client disconnected");
        break;
      default:
        break;
    }
  });

  logs.append("HTTP server started");
  Serial.println("HTTP server started");
  if (WiFi.status() == WL_CONNECTED) {
    Serial.printf("Open http://%s or http://%s.local\n", WiFi.localIP().toString().c_str(), config.deviceName.c_str());
  }
}

void AppServer::loop() {
  sensors.update();
  server.handleClient();
  webSocket.loop();
  broadcastStatusIfDue();
  maybeMaintainWifi();
}

void AppServer::connectWifi() {
  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false);

  if (config.wifiSsid.isEmpty()) {
    logs.append("No Wi-Fi SSID configured");
    return;
  }

  WiFi.begin(config.wifiSsid.c_str(), config.wifiPassword.c_str());
  logs.append("Connecting Wi-Fi to SSID: " + config.wifiSsid);

  uint32_t start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 15000) {
    delay(250);
  }

  if (WiFi.status() == WL_CONNECTED) {
    logs.append("Wi-Fi connected: " + WiFi.localIP().toString());
  } else {
    logs.append("Wi-Fi connect timeout");
  }
}

void AppServer::maybeMaintainWifi() {
  if (millis() - lastWifiCheckMs < 10000) return;
  lastWifiCheckMs = millis();

  if (WiFi.status() != WL_CONNECTED && !config.wifiSsid.isEmpty()) {
    logs.append("Wi-Fi reconnect attempt");
    WiFi.disconnect();
    WiFi.begin(config.wifiSsid.c_str(), config.wifiPassword.c_str());
  }
}

void AppServer::startMdns() {
  if (WiFi.status() == WL_CONNECTED) {
    if (MDNS.begin(config.deviceName.c_str())) {
      logs.append("mDNS started: " + config.deviceName + ".local");
    } else {
      logs.append("mDNS failed");
    }
  }
}

String AppServer::getContentType(const String& path) {
  if (path.endsWith(".html")) return "text/html";
  if (path.endsWith(".css")) return "text/css";
  if (path.endsWith(".js")) return "application/javascript";
  if (path.endsWith(".json")) return "application/json";
  if (path.endsWith(".txt")) return "text/plain";
  if (path.endsWith(".svg")) return "image/svg+xml";
  return "application/octet-stream";
}

bool AppServer::serveFile(String path) {
  if (path.endsWith("/")) path += "index.html";
  if (!LittleFS.exists(path)) return false;
  File f = LittleFS.open(path, FILE_READ);
  if (!f) return false;
  server.streamFile(f, getContentType(path));
  f.close();
  return true;
}

bool AppServer::requirePasswordFromJson() {
  if (config.adminPassword.isEmpty()) return true;
  if (!server.hasArg("plain")) return false;
  StaticJsonDocument<128> doc;
  if (deserializeJson(doc, server.arg("plain"))) return false;
  const char* supplied = doc["password"] | "";
  return String(supplied) == config.adminPassword;
}

String AppServer::getStatusJson() {
  StaticJsonDocument<768> doc;
  auto sensor = sensors.snapshot();
  auto stat = stats.snapshot();

  doc["deviceName"] = config.deviceName;
  doc["wifiConnected"] = (WiFi.status() == WL_CONNECTED);
  doc["ssid"] = config.wifiSsid;
  doc["ip"] = (WiFi.status() == WL_CONNECTED) ? WiFi.localIP().toString() : "";
  doc["rssi"] = (WiFi.status() == WL_CONNECTED) ? WiFi.RSSI() : 0;
  doc["uptimeMs"] = millis();
  doc["freeHeap"] = ESP.getFreeHeap();
  doc["mdns"] = config.deviceName + ".local";
  doc["bootCount"] = stat.bootCount;
  doc["configSaveCount"] = stat.configSaveCount;
  doc["otaUpdateCount"] = stat.otaUpdateCount;
  doc["analogRaw"] = sensor.analogRaw;
  doc["analogVolts"] = sensor.analogVolts;
  doc["sampleCount"] = sensor.sampleCount;
  doc["sampleIntervalMs"] = config.sampleIntervalMs;
  doc["logSizeBytes"] = logs.size();

  String out;
  serializeJson(doc, out);
  return out;
}

String AppServer::getConfigJson() {
  StaticJsonDocument<256> doc;
  doc["wifiSsid"] = config.wifiSsid;
  doc["deviceName"] = config.deviceName;
  doc["sampleIntervalMs"] = config.sampleIntervalMs;
  doc["analogPin"] = config.analogPin;
  String out;
  serializeJson(doc, out);
  return out;
}

String AppServer::getStatsJson() {
  StaticJsonDocument<256> doc;
  auto stat = stats.snapshot();
  doc["bootCount"] = stat.bootCount;
  doc["configSaveCount"] = stat.configSaveCount;
  doc["otaUpdateCount"] = stat.otaUpdateCount;
  doc["logSizeBytes"] = logs.size();
  String out;
  serializeJson(doc, out);
  return out;
}

String AppServer::getSensorJson() {
  StaticJsonDocument<256> doc;
  auto sensor = sensors.snapshot();
  doc["analogRaw"] = sensor.analogRaw;
  doc["analogVolts"] = sensor.analogVolts;
  doc["sampleCount"] = sensor.sampleCount;
  doc["lastSampleAgeMs"] = sensor.lastSampleAgeMs;
  doc["analogPin"] = config.analogPin;
  doc["sampleIntervalMs"] = config.sampleIntervalMs;
  String out;
  serializeJson(doc, out);
  return out;
}

void AppServer::registerRoutes() {
  server.on("/", HTTP_GET, [this]() {
    if (!serveFile("/index.html")) {
      server.send(500, "text/plain", "Missing index.html");
    }
  });

  server.on("/api/status", HTTP_GET, [this]() {
    server.send(200, "application/json", getStatusJson());
  });

  server.on("/api/config", HTTP_GET, [this]() {
    server.send(200, "application/json", getConfigJson());
  });

  server.on("/api/stats", HTTP_GET, [this]() {
    server.send(200, "application/json", getStatsJson());
  });

  server.on("/api/sensors", HTTP_GET, [this]() {
    server.send(200, "application/json", getSensorJson());
  });

  server.on("/api/config", HTTP_POST, [this]() {
    if (!server.hasArg("plain")) {
      server.send(400, "application/json", "{\"ok\":false,\"error\":\"missing body\"}");
      return;
    }

    StaticJsonDocument<512> doc;
    auto err = deserializeJson(doc, server.arg("plain"));
    if (err) {
      server.send(400, "application/json", "{\"ok\":false,\"error\":\"invalid json\"}");
      return;
    }

    String suppliedPassword = String((const char*)doc["password"] | "");
    if (!config.adminPassword.isEmpty() && suppliedPassword != config.adminPassword) {
      server.send(403, "application/json", "{\"ok\":false,\"error\":\"bad password\"}");
      return;
    }

    config.wifiSsid = String((const char*)doc["wifiSsid"] | config.wifiSsid);
    config.wifiPassword = String((const char*)doc["wifiPassword"] | config.wifiPassword);
    config.deviceName = String((const char*)doc["deviceName"] | config.deviceName);
    config.sampleIntervalMs = doc["sampleIntervalMs"] | config.sampleIntervalMs;
    config.analogPin = doc["analogPin"] | config.analogPin;

    if (doc.containsKey("newAdminPassword")) {
      config.adminPassword = String((const char*)doc["newAdminPassword"] | config.adminPassword);
    }

    sensors.applyConfig(config.analogPin, config.sampleIntervalMs);

    if (!configManager.save(config)) {
      server.send(500, "application/json", "{\"ok\":false,\"error\":\"save failed\"}");
      return;
    }

    stats.incrementConfigSaveCount();
    logs.append("Config updated via API");
    server.send(200, "application/json", "{\"ok\":true,\"message\":\"saved; reboot recommended for Wi-Fi/device name changes\"}");
  });

  server.on("/api/restart", HTTP_POST, [this]() {
    if (!requirePasswordFromJson()) {
      server.send(403, "application/json", "{\"ok\":false,\"error\":\"bad password\"}");
      return;
    }
    server.send(200, "application/json", "{\"ok\":true,\"message\":\"restarting\"}");
    logs.append("Restart requested via API");
    delay(300);
    ESP.restart();
  });

  server.on("/logs.txt", HTTP_GET, [this]() {
    server.send(200, "text/plain", logs.readAll());
  });

  server.onNotFound([this]() {
    if (!serveFile(server.uri())) {
      server.send(404, "text/plain", "404 not found");
    }
  });
}

void AppServer::registerSocket() {
  // Initialization is handled in begin().
}

void AppServer::broadcastStatusIfDue() {
  if (millis() - lastBroadcastMs < config.sampleIntervalMs) return;
  lastBroadcastMs = millis();
  webSocket.broadcastTXT(getStatusJson());
}
