#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <ESPmDNS.h>
#include <Preferences.h>

#include "Config.h"
#include "ConfigManager.h"
#include "LogManager.h"
#include "StatsManager.h"
#include "SensorManager.h"
#include "OtaManager.h"

class AppServer {
public:
  void begin();
  void loop();

private:
  WebServer server{80};
  WebSocketsServer webSocket{81};

  AppConfig config;
  ConfigManager configManager;
  LogManager logs;
  StatsManager stats;
  SensorManager sensors;
  OtaManager ota;

  uint32_t lastBroadcastMs = 0;
  uint32_t lastWifiCheckMs = 0;

  void connectWifi();
  void maybeMaintainWifi();
  void startMdns();
  void registerRoutes();
  void registerSocket();
  void broadcastStatusIfDue();

  String getStatusJson();
  String getConfigJson();
  String getStatsJson();
  String getSensorJson();
  String getContentType(const String& path);
  bool serveFile(String path);
  bool requirePasswordFromJson();
};
