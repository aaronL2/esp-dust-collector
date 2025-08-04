#include "config_ui.h"
#include <ArduinoJson.h>

Preferences prefs;
AsyncWebServer server(80);
ConfigUIClass configUI;

void handleConfigPost(AsyncWebServerRequest *request) {
  if (request->contentType() != "application/json") {
    request->send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
    return;
  }

  request->send(200, "application/json", "{\"status\":\"ok\"}");
}

void ConfigUIClass::begin() {
  prefs.begin("config", false);
  friendlyName = prefs.getString("friendly_name", "station");
  baseMac = prefs.getString("base_mac", "");

  server.on("/config", HTTP_POST, handleConfigPost);
  server.begin();
}

String ConfigUIClass::getFriendlyName() {
  return friendlyName;
}

String ConfigUIClass::getBaseMac() {
  return baseMac;
}
