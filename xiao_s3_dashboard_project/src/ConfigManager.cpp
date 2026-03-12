#include "ConfigManager.h"

bool ConfigManager::begin() {
  return ensureDefaultConfig();
}

void ConfigManager::setDefaults(AppConfig& config) {
  config.wifiSsid = "YOUR_WIFI_SSID";
  config.wifiPassword = "YOUR_WIFI_PASSWORD";
  config.deviceName = "xiao-s3";
  config.adminPassword = "admin";
  config.sampleIntervalMs = 1000;
  config.analogPin = 1;
}

bool ConfigManager::ensureDefaultConfig() {
  if (LittleFS.exists(kConfigPath)) {
    return true;
  }

  AppConfig cfg;
  setDefaults(cfg);
  return save(cfg);
}

bool ConfigManager::load(AppConfig& config) {
  if (!LittleFS.exists(kConfigPath)) {
    setDefaults(config);
    return false;
  }

  File f = LittleFS.open(kConfigPath, FILE_READ);
  if (!f) {
    setDefaults(config);
    return false;
  }

  StaticJsonDocument<512> doc;
  auto err = deserializeJson(doc, f);
  f.close();

  if (err) {
    setDefaults(config);
    return false;
  }

  config.wifiSsid = String((const char*)doc["wifiSsid"] | "YOUR_WIFI_SSID");
  config.wifiPassword = String((const char*)doc["wifiPassword"] | "YOUR_WIFI_PASSWORD");
  config.deviceName = String((const char*)doc["deviceName"] | "xiao-s3");
  config.adminPassword = String((const char*)doc["adminPassword"] | "admin");
  config.sampleIntervalMs = doc["sampleIntervalMs"] | 1000;
  config.analogPin = doc["analogPin"] | 1;
  return true;
}

bool ConfigManager::save(const AppConfig& config) {
  StaticJsonDocument<512> doc;
  doc["wifiSsid"] = config.wifiSsid;
  doc["wifiPassword"] = config.wifiPassword;
  doc["deviceName"] = config.deviceName;
  doc["adminPassword"] = config.adminPassword;
  doc["sampleIntervalMs"] = config.sampleIntervalMs;
  doc["analogPin"] = config.analogPin;

  File f = LittleFS.open(kConfigPath, FILE_WRITE);
  if (!f) {
    return false;
  }

  bool ok = serializeJsonPretty(doc, f) > 0;
  f.close();
  return ok;
}
