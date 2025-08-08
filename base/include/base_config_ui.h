#ifndef BASE_CONFIG_UI_H
#define BASE_CONFIG_UI_H

#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>

struct BaseConfig {
  char friendlyName[32];
  char baseMac[18];
};

extern BaseConfig config;
void setupBaseConfigUI(AsyncWebServer& server);

#endif // BASE_CONFIG_UI_H
