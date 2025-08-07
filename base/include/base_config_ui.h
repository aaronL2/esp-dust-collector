#pragma once
#include <Arduino.h>
#include <ESPAsyncWebServer.h>

namespace configUI {
  void loadConfig();
  void begin(AsyncWebServer& server);

  String getFriendlyName();
  String getWifiSSID();
  String getWifiPassword();
}
