#include "StatsManager.h"

bool StatsManager::begin() {
  ready = prefs.begin("dashboard", false);
  return ready;
}

void StatsManager::incrementBootCount() {
  if (!ready) return;
  uint32_t count = prefs.getUInt("bootCount", 0);
  prefs.putUInt("bootCount", count + 1);
}

void StatsManager::incrementConfigSaveCount() {
  if (!ready) return;
  uint32_t count = prefs.getUInt("cfgSaves", 0);
  prefs.putUInt("cfgSaves", count + 1);
}

void StatsManager::incrementOtaUpdateCount() {
  if (!ready) return;
  uint32_t count = prefs.getUInt("otaCount", 0);
  prefs.putUInt("otaCount", count + 1);
}

StatsSnapshot StatsManager::snapshot() const {
  StatsSnapshot s{};
  if (!ready) return s;
  s.bootCount = prefs.getUInt("bootCount", 0);
  s.configSaveCount = prefs.getUInt("cfgSaves", 0);
  s.otaUpdateCount = prefs.getUInt("otaCount", 0);
  return s;
}
