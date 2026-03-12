#include "LogManager.h"

bool LogManager::begin() {
  if (!LittleFS.exists(kLogPath)) {
    File f = LittleFS.open(kLogPath, FILE_WRITE);
    if (!f) return false;
    f.println("Log created");
    f.close();
  }
  rotateIfNeeded();
  return true;
}

void LogManager::rotateIfNeeded() {
  File f = LittleFS.open(kLogPath, FILE_READ);
  if (!f) return;
  size_t sz = f.size();
  if (sz <= kMaxSizeBytes) {
    f.close();
    return;
  }

  size_t skip = (sz > kKeepTailBytes) ? (sz - kKeepTailBytes) : 0;
  f.seek(skip, SeekSet);
  String tail = f.readString();
  f.close();

  File out = LittleFS.open(kLogPath, FILE_WRITE);
  if (!out) return;
  out.println("--- log rotated ---");
  out.print(tail);
  out.close();
}

void LogManager::append(const String& line) {
  rotateIfNeeded();
  File f = LittleFS.open(kLogPath, FILE_APPEND);
  if (!f) return;
  f.printf("[%10lu ms] %s\n", millis(), line.c_str());
  f.close();
}

String LogManager::readAll() {
  File f = LittleFS.open(kLogPath, FILE_READ);
  if (!f) return "Could not open logs";
  String out = f.readString();
  f.close();
  return out;
}

size_t LogManager::size() const {
  File f = LittleFS.open(kLogPath, FILE_READ);
  if (!f) return 0;
  size_t sz = f.size();
  f.close();
  return sz;
}
