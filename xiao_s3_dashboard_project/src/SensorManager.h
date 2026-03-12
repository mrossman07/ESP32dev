#pragma once

#include <Arduino.h>

struct SensorSnapshot {
  uint16_t analogRaw;
  float analogVolts;
  uint32_t sampleCount;
  uint32_t lastSampleAgeMs;
};

class SensorManager {
public:
  void begin(uint8_t analogPin, uint16_t sampleIntervalMs);
  void update();
  SensorSnapshot snapshot() const;
  void applyConfig(uint8_t analogPin, uint16_t sampleIntervalMs);

private:
  uint8_t m_analogPin = 1;
  uint16_t m_intervalMs = 1000;
  uint32_t m_lastSampleMs = 0;
  uint32_t m_sampleCount = 0;
  uint16_t m_lastRaw = 0;
  float m_lastVolts = 0.0f;
};
