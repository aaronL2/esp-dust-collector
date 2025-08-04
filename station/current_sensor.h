#ifndef CURRENT_SENSOR_H
#define CURRENT_SENSOR_H

class CurrentSensorClass {
public:
  void begin();
  float read();

private:
  const int sensorPin = 34;  // Example ADC pin for ESP32
  const float calibration = 30.0;  // Adjust for SCT-013-000 scaling
};

extern CurrentSensorClass CurrentSensor;

#endif
