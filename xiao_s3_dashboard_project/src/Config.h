#pragma once

#include <Arduino.h>

struct AppConfig {
  String wifiSsid;
  String wifiPassword;
  String deviceName;
  String adminPassword;
  uint16_t sampleIntervalMs;
  uint8_t analogPin;
};
