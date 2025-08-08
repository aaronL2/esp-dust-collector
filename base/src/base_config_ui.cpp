
#include "base_config_ui.h"
#include <SPIFFS.h>

BaseConfigUI configUI;

void BaseConfigUI::begin(AsyncWebServer &server) {
  // Implement your configuration UI routes if needed
}

void BaseConfigUI::loadConfig() {
  if (!SPIFFS.begin(true)) return;

  File file = SPIFFS.open("/config.json");
  if (!file) return;

  DeserializationError error = deserializeJson(config, file);
  if (error) {
    Serial.println("Failed to load config");
  }
  file.close();
}

void BaseConfigUI::saveConfig() {
  File file = SPIFFS.open("/config.json", FILE_WRITE);
  if (!file) return;

  serializeJson(config, file);
  file.close();
}

void BaseConfigUI::setWifiSSID(const String& ssid) {
  config["wifi_ssid"] = ssid;
}

void BaseConfigUI::setWifiPassword(const String& password) {
  config["wifi_password"] = password;
}

String BaseConfigUI::getWifiSSID() {
  return config["wifi_ssid"] | "";
}

String BaseConfigUI::getWifiPassword() {
  return config["wifi_password"] | "";
}

String BaseConfigUI::getFriendlyName() {
  return config["name"] | "dustbase";
}

String BaseConfigUI::getBaseMac() {
  return config["base_mac"] | "";
}
