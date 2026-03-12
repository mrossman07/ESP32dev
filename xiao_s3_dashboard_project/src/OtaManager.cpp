#include "OtaManager.h"

void OtaManager::begin(LogManager* logManager, StatsManager* statsManager, const String* adminPassword) {
  logs = logManager;
  stats = statsManager;
  password = adminPassword;
}

bool OtaManager::authorized(WebServer& server) {
  if (!password || password->isEmpty()) return true;
  if (!server.hasArg("password")) return false;
  return server.arg("password") == *password;
}

String OtaManager::htmlPage() {
  return R"HTML(
<!doctype html>
<html>
<head><meta name="viewport" content="width=device-width,initial-scale=1"><title>OTA Update</title>
<style>body{font-family:Arial;margin:2rem;max-width:700px}input,button{padding:.7rem;margin:.4rem 0}pre{background:#111;color:#d7ffd7;padding:1rem;border-radius:8px}</style>
</head>
<body>
<h1>OTA Update</h1>
<p>Select a compiled <code>.bin</code> firmware file. Enter the admin password from <code>config.json</code>.</p>
<form method="POST" action="/api/ota" enctype="multipart/form-data">
  <label>Password</label><br>
  <input type="password" name="password"><br>
  <input type="file" name="firmware"><br>
  <button type="submit">Upload Firmware</button>
</form>
<p><a href="/">Back to dashboard</a></p>
</body></nhtml>)HTML";
}

void OtaManager::registerRoutes(WebServer& server) {
  server.on("/ota", HTTP_GET, [this, &server]() {
    server.send(200, "text/html", htmlPage());
  });

  server.on(
    "/api/ota", HTTP_POST,
    [this, &server]() {
      if (!authorized(server)) {
        server.send(403, "application/json", "{\"ok\":false,\"error\":\"bad password\"}");
        return;
      }

      if (Update.hasError()) {
        server.send(500, "application/json", "{\"ok\":false,\"error\":\"update failed\"}");
        if (logs) logs->append("OTA failed");
      } else {
        if (stats) stats->incrementOtaUpdateCount();
        if (logs) logs->append("OTA success; rebooting");
        server.send(200, "application/json", "{\"ok\":true,\"message\":\"update successful, rebooting\"}");
        delay(500);
        ESP.restart();
      }
    },
    [this, &server]() {
      HTTPUpload& upload = server.upload();

      if (upload.status == UPLOAD_FILE_START) {
        if (!authorized(server)) return;
        if (logs) logs->append("OTA upload start: " + upload.filename);
        if (!Update.begin(UPDATE_SIZE_UNKNOWN)) {
          Update.printError(Serial);
        }
      } else if (upload.status == UPLOAD_FILE_WRITE) {
        if (!authorized(server)) return;
        if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
          Update.printError(Serial);
        }
      } else if (upload.status == UPLOAD_FILE_END) {
        if (!authorized(server)) return;
        if (!Update.end(true)) {
          Update.printError(Serial);
        }
      } else if (upload.status == UPLOAD_FILE_ABORTED) {
        Update.abort();
        if (logs) logs->append("OTA upload aborted");
      }
    }
  );
}
