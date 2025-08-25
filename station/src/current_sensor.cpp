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
  unsigned long start = millis();
  double sum = 0.0;
  int count = 0;
  while (millis() - start < static_cast<unsigned long>(sampleWindowMs)) {
    float raw = analogRead(sensorPin);
    float diff = raw - zeroOffset;
    sum += diff * diff;
    ++count;
  }
  if (count == 0) {
    return rms;  // Avoid division by zero
  }
  float newRms = sqrt(sum / count) * (3.3f / 4095.0f) * calibration;
  rms = 0.8f * rms + 0.2f * newRms;
  const float noiseThreshold = 0.05f;  // Ignore small noise readings
  if (abs(rms) < noiseThreshold) {
    return 0.0f;
  }
  return rms;
}

void CurrentSensorClass::recalibrate() {
  const int samples = 100;
  long total = 0;
  for (int i = 0; i < samples; ++i) {
    total += analogRead(sensorPin);
  }
  zeroOffset = total / static_cast<float>(samples);
}
