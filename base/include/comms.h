#pragma once
#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>

void comms_setup();
void comms_loop();
void setStationOffDelay(const String& mac, float offDelay);
void setStationThreshold(const String& mac, float threshold);
float getStationCurrent(const String& mac);
void sendRecalibrateCommand(const String& mac);
