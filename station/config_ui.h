#pragma once
#include <Arduino.h>
#include <Preferences.h>
#include <ESPAsyncWebServer.h>

class ConfigUIClass {
public:
  void begin();
  String getFriendlyName();
  String getBaseMac();
private:
  String friendlyName;
  String baseMac;
};

extern ConfigUIClass configUI;
