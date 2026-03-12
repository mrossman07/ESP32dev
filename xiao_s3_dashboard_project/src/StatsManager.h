#pragma once

#include <Arduino.h>
#include <Preferences.h>

struct StatsSnapshot {
  uint32_t bootCount;
  uint32_t configSaveCount;
  uint32_t otaUpdateCount;
};

class StatsManager {
public:
  bool begin();
  void incrementBootCount();
  void incrementConfigSaveCount();
  void incrementOtaUpdateCount();
  StatsSnapshot snapshot() const;

private:
  mutable Preferences prefs;
  bool ready = false;
};
