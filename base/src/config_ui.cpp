#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>
#include "config_ui.h"

extern AsyncWebServer server;   // defined in base/src/main.cpp

BaseConfigUI configUI;

// --- shared free-function API (base side) ---
String getFriendlyName() { return configUI.getFriendlyName(); }
String getBaseMac()      { return configUI.getBaseMac(); }
String getMdnsName() {
  String n = configUI.getFriendlyName();
  n.replace(" ", "");
  return n;
}
void configUI_setup() { configUI.begin(server); }

// --- BaseConfigUI methods ---
void BaseConfigUI::begin(AsyncWebServer& server) {
  // optional: register base-specific routes here later
}

void BaseConfigUI::loadConfig() {
  if (!SPIFFS.begin(true)) return;
  File file = SPIFFS.open("/config.json");
  if (!file) return;
  DeserializationError err = deserializeJson(config, file);
  if (err) Serial.println("Failed to load config");
  file.close();
}

void BaseConfigUI::saveConfig() {
  File file = SPIFFS.open("/config.json", FILE_WRITE);
  if (!file) return;
  serializeJson(config, file);
  file.close();
}

void BaseConfigUI::setWifiSSID(const String& ssid)  {
   config["wifi_ssid"] = ssid; 
}

void BaseConfigUI::setWifiPassword(const String& pass)  {
   config["wifi_password"] = pass; 
}

String BaseConfigUI::getWifiSSID() const  {
   return config["wifi_ssid"] | ""; 
}

String BaseConfigUI::getWifiPassword() const  {
   return config["wifi_password"] | ""; 
}

String BaseConfigUI::getFriendlyName() const  {
   return config["name"] | "dustbase"; 
}

String BaseConfigUI::getBaseMac() const {
   return config["base_mac"] | ""; 
}
