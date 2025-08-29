#include "current_sensor.h"
#include <Arduino.h>

CurrentSensorClass CurrentSensor;

void CurrentSensorClass::begin() {
  pinMode(sensorPin, INPUT);
  analogSetPinAttenuation(sensorPin, ADC_11db);  // 0-3.3V range
  analogReadResolution(12);  // ESP32 default
  recalibrate();
}

float CurrentSensorClass::read() {
  const unsigned long sampleWindowMs = 200;
  unsigned long start = millis();
  float sumSquares = 0.0f;
  uint16_t count = 0;
  while (millis() - start < sampleWindowMs) {
    float raw = analogRead(sensorPin) - zeroOffset;
    sumSquares += raw * raw;
    ++count;
  }
  float newRms = 0.0f;
  if (count > 0) {
    newRms = sqrt(sumSquares / count) * (3.3f / 4095.0f) * calibration;
  }
  newRms = max(newRms - baselineRms, 0.0f);

  const float noiseThreshold = 0.05f;  // Ignore small noise readings
  if (newRms < noiseThreshold) {
    return 0.0f;
  }
  return newRms;
}

void CurrentSensorClass::recalibrate() {
  const unsigned long sampleWindowMs = 200;
  unsigned long start = millis();
  double sum = 0.0;
  double sumSquares = 0.0;
  uint16_t count = 0;
  while (millis() - start < sampleWindowMs) {
    float raw = analogRead(sensorPin);
    sum += raw;
    sumSquares += raw * raw;
    ++count;
  }
  if (count > 0) {
    zeroOffset = sum / count;
    double meanSq = sumSquares / count;
    double variance = meanSq - (zeroOffset * zeroOffset);
    if (variance < 0) {
      variance = 0;
    }
    double rms = sqrt(variance);
    double voltage = rms * (3.3 / 4095.0);
    baselineRms = voltage * calibration;
  } else {
    baselineRms = 0.0f;
  }
}
