#include "SensorManager.h"

#ifndef ADC_MAX_VALUE
#define ADC_MAX_VALUE 4095.0f
#endif

void SensorManager::begin(uint8_t analogPin, uint16_t sampleIntervalMs) {
  m_analogPin = analogPin;
  m_intervalMs = sampleIntervalMs;
  pinMode(m_analogPin, INPUT);
  analogReadResolution(12);
  update();
}

void SensorManager::applyConfig(uint8_t analogPin, uint16_t sampleIntervalMs) {
  m_analogPin = analogPin;
  m_intervalMs = sampleIntervalMs;
  pinMode(m_analogPin, INPUT);
}

void SensorManager::update() {
  uint32_t now = millis();
  if (now - m_lastSampleMs < m_intervalMs && m_sampleCount > 0) {
    return;
  }

  m_lastSampleMs = now;
  m_lastRaw = analogRead(m_analogPin);
  m_lastVolts = (3.3f * m_lastRaw) / ADC_MAX_VALUE;
  ++m_sampleCount;
}

SensorSnapshot SensorManager::snapshot() const {
  SensorSnapshot s{};
  s.analogRaw = m_lastRaw;
  s.analogVolts = m_lastVolts;
  s.sampleCount = m_sampleCount;
  s.lastSampleAgeMs = millis() - m_lastSampleMs;
  return s;
}
