#include "current_sensor.h"
#include <Arduino.h>

CurrentSensorClass CurrentSensor;

void CurrentSensorClass::begin() {
  analogReadResolution(12);  // ESP32 default
}

float CurrentSensorClass::read() {
  int raw = analogRead(sensorPin);
  float voltage = raw * (3.3 / 4095.0);  // Scale to 3.3V reference
  float amps = voltage * calibration;   // Simplified conversion
  return amps;
}
