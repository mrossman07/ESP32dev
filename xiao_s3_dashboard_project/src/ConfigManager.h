#pragma once

#include <Arduino.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include "Config.h"

class ConfigManager {
public:
  static constexpr const char* kConfigPath = "/config.json";

  bool begin();
  bool load(AppConfig& config);
  bool save(const AppConfig& config);
  void setDefaults(AppConfig& config);

private:
  bool ensureDefaultConfig();
};
