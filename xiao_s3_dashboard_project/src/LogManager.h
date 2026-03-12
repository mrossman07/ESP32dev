#pragma once

#include <Arduino.h>
#include <LittleFS.h>

class LogManager {
public:
  static constexpr const char* kLogPath = "/logs.txt";
  static constexpr size_t kMaxSizeBytes = 24 * 1024;
  static constexpr size_t kKeepTailBytes = 12 * 1024;

  bool begin();
  void append(const String& line);
  String readAll();
  size_t size() const;

private:
  void rotateIfNeeded();
};
