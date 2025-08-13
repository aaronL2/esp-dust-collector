#pragma once
#include <Arduino.h>
#include <ArduinoJson.h>      // for JsonDocument
class AsyncWebServer;

class BaseConfigUI {
public:
  void begin(AsyncWebServer& server);

  void loadConfig();
  void saveConfig();

  void   setWifiSSID(const String& ssid);
  void   setWifiPassword(const String& password);
  String getWifiSSID() const;
  String getWifiPassword() const;

  String getFriendlyName() const;
  String getBaseMac() const;

private:
  JsonDocument config;  // holds {"wifi_ssid","wifi_password","name","base_mac",...}
};

// Global instance (defined in config_ui.cpp)
extern BaseConfigUI configUI;

// Free-function API shared across base & station (declarations only)
String getFriendlyName();
String getBaseMac();
String getMdnsName();
void   configUI_setup();
