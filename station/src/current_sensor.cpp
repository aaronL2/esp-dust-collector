#include "current_sensor.h"
#include <Arduino.h>

CurrentSensorClass CurrentSensor;

void CurrentSensorClass::begin() {
  pinMode(sensorPin, INPUT);
  analogSetPinAttenuation(sensorPin, ADC_11db);  // 0-3.3V range
  analogReadResolution(12);  // ESP32 default
}

float CurrentSensorClass::read() {
  const int samples = 10;
  long total = 0;
  for (int i = 0; i < samples; ++i) {
    total += analogRead(sensorPin);
  }
  float raw = total / static_cast<float>(samples);
  float voltage = (raw - 2048.0) * (3.3 / 4095.0);  // Remove midpoint bias
  float amps = voltage * calibration;
  const float noiseThreshold = 0.05;  // Ignore small noise readings
  if (abs(amps) < noiseThreshold) {
    return 0.0;
  }
  return amps;
}