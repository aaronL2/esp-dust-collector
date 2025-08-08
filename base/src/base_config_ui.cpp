// === base_config_ui.cpp (Updated for ArduinoJson 7+) ===

#include <ArduinoJson.h>
#include <SPIFFS.h>
#include <FS.h>
#include "base_config_ui.h"
BaseConfig config;

const char* configPath = "/config.json";

void loadConfig(BaseConfig& config) {
  File file = SPIFFS.open(configPath, "r");
  if (!file) {
    Serial.println("[CONFIG] Failed to open config file");
    return;
  }

  ArduinoJson::JsonDocument doc;
  DeserializationError error = deserializeJson(doc, file);
  if (error) {
    Serial.println("[CONFIG] Failed to parse config file");
    return;
  }

  if (doc["friendlyName"].is<const char*>()) {
    strlcpy(config.friendlyName, doc["friendlyName"], sizeof(config.friendlyName));
  }

  if (doc["baseMac"].is<const char*>()) {
    strlcpy(config.baseMac, doc["baseMac"], sizeof(config.baseMac));
  }

  file.close();
}

void saveConfig(const BaseConfig& config) {
  File file = SPIFFS.open(configPath, "w");
  if (!file) {
    Serial.println("[CONFIG] Failed to open config file for writing");
    return;
  }

  ArduinoJson::JsonDocument doc;
  doc["friendlyName"] = config.friendlyName;
  doc["baseMac"] = config.baseMac;

  serializeJsonPretty(doc, file);
  file.close();
}

void setupConfigUI(AsyncWebServer& server, BaseConfig& config) {
  server.on("/config", HTTP_GET, [&config](AsyncWebServerRequest *request){
    String html = "<form method='POST' action='/save'>";
    html += "<label for='friendlyName'>Friendly Name:</label><br>";
    html += "<input type='text' id='friendlyName' name='friendlyName' value='" + String(config.friendlyName) + "'><br>";
    html += "<label for='baseMac'>Base MAC:</label><br>";
    html += "<input type='text' id='baseMac' name='baseMac' value='" + String(config.baseMac) + "'><br>";
    html += "<input type='submit' value='Save'>";
    html += "</form>";
    request->send(200, "text/html", html);
  });

  server.on("/save", HTTP_POST, [&config](AsyncWebServerRequest *request){
    if (request->hasParam("friendlyName", true)) {
      strlcpy(config.friendlyName, request->getParam("friendlyName", true)->value().c_str(), sizeof(config.friendlyName));
    }
    if (request->hasParam("baseMac", true)) {
      strlcpy(config.baseMac, request->getParam("baseMac", true)->value().c_str(), sizeof(config.baseMac));
    }
    saveConfig(config);
    request->send(200, "text/plain", "Config saved. Rebooting...");
    delay(1000);
    ESP.restart();
  });
}
