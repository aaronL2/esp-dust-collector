#ifndef BASE_CONFIG_UI_H
#define BASE_CONFIG_UI_H

#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>

// ---- Persistent config ----
struct BaseConfig {
  char friendlyName[32] = "Dust Base";
  char baseMac[18]      = "";
  char wifiSsid[33]     = "";
  char wifiPassword[65] = "";
};

extern BaseConfig config;

// Register routes (GET /config, POST /save)
void setupBaseConfigUI(AsyncWebServer& server);

// Back-compat wrapper that exposes expected methods used by main.cpp
class BaseConfigUI {
public:
  // Add routes into server
  void setup(AsyncWebServer& s) { setupBaseConfigUI(s); }
  void begin(AsyncWebServer& s) { setupBaseConfigUI(s); }
  void routes(AsyncWebServer& s){ setupBaseConfigUI(s); }

  // Load config from SPIFFS
  bool loadConfig();

  // Accessors used by main.cpp
  const char* getWifiSSID() const      { return config.wifiSsid; }
  const char* getWifiPassword() const  { return config.wifiPassword; }
  const char* getFriendlyName() const  { return config.friendlyName; }

  // Optional: save (used internally by the UI)
  bool saveConfig();
};

extern BaseConfigUI configUI;

#endif // BASE_CONFIG_UI_H
