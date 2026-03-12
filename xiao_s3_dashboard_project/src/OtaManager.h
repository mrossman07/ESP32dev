#pragma once

#include <Arduino.h>
#include <WebServer.h>
#include <Update.h>
#include "LogManager.h"
#include "StatsManager.h"

class OtaManager {
public:
  void begin(LogManager* logManager, StatsManager* statsManager, const String* adminPassword);
  void registerRoutes(WebServer& server);

private:
  LogManager* logs = nullptr;
  StatsManager* stats = nullptr;
  const String* password = nullptr;

  bool authorized(WebServer& server);
  static String htmlPage();
};
