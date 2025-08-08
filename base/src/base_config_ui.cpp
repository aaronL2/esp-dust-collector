#include "base_config_ui.h"
#include <SPIFFS.h>
#include <FS.h>

static const char* kConfigPath = "/config.json";

// define globals
BaseConfig   config;
BaseConfigUI configUI;

// ---- helpers (non-member) ----
static bool loadConfigInternal() {
  if (!SPIFFS.exists(kConfigPath)) return false;
  File f = SPIFFS.open(kConfigPath, "r");
  if (!f) return false;

  ArduinoJson::JsonDocument doc;
  auto err = deserializeJson(doc, f);
  f.close();
  if (err) return false;

  if (doc["friendlyName"].is<const char*>())
    strlcpy(config.friendlyName, doc["friendlyName"], sizeof(config.friendlyName));
  if (doc["baseMac"].is<const char*>())
    strlcpy(config.baseMac, doc["baseMac"], sizeof(config.baseMac));
  if (doc["wifiSsid"].is<const char*>())
    strlcpy(config.wifiSsid, doc["wifiSsid"], sizeof(config.wifiSsid));
  if (doc["wifiPassword"].is<const char*>())
    strlcpy(config.wifiPassword, doc["wifiPassword"], sizeof(config.wifiPassword));

  return true;
}

static bool saveConfigInternal() {
  File f = SPIFFS.open(kConfigPath, "w");
  if (!f) return false;

  ArduinoJson::JsonDocument doc;
  doc["friendlyName"] = config.friendlyName;
  doc["baseMac"]      = config.baseMac;
  doc["wifiSsid"]     = config.wifiSsid;
  doc["wifiPassword"] = config.wifiPassword;

  serializeJsonPretty(doc, f);
  f.close();
  return true;
}

// ---- BaseConfigUI members ----
bool BaseConfigUI::loadConfig()  { return loadConfigInternal(); }
bool BaseConfigUI::saveConfig()  { return saveConfigInternal(); }

// ---- routes ----
void setupBaseConfigUI(AsyncWebServer& server) {
  // Ensure config is loaded once at boot
  configUI.loadConfig();

  server.on("/config", HTTP_GET, [](AsyncWebServerRequest* req){
    String html;
    html.reserve(2048);
    html += F("<html><body><h2>Base Config</h2><form method='POST' action='/save'>");

    html += F("<label>Friendly Name:</label><br>");
    html += "<input name='friendlyName' value='" + String(config.friendlyName) + "'><br><br>";

    html += F("<label>Base MAC:</label><br>");
    html += "<input name='baseMac' value='" + String(config.baseMac) + "'><br><br>";

    html += F("<label>WiFi SSID:</label><br>");
    html += "<input name='wifiSsid' value='" + String(config.wifiSsid) + "'><br><br>";

    html += F("<label>WiFi Password:</label><br>");
    html += "<input name='wifiPassword' type='password' value='" + String(config.wifiPassword) + "'><br><br>";

    html += F("<input type='submit' value='Save'></form></body></html>");
    req->send(200, "text/html", html);
  });

  server.on("/save", HTTP_POST, [](AsyncWebServerRequest* req){
    if (req->hasParam("friendlyName", true))
      strlcpy(config.friendlyName, req->getParam("friendlyName", true)->value().c_str(), sizeof(config.friendlyName));
    if (req->hasParam("baseMac", true))
      strlcpy(config.baseMac, req->getParam("baseMac", true)->value().c_str(), sizeof(config.baseMac));
    if (req->hasParam("wifiSsid", true))
      strlcpy(config.wifiSsid, req->getParam("wifiSsid", true)->value().c_str(), sizeof(config.wifiSsid));
    if (req->hasParam("wifiPassword", true))
      strlcpy(config.wifiPassword, req->getParam("wifiPassword", true)->value().c_str(), sizeof(config.wifiPassword));

    configUI.saveConfig();
    req->send(200, "text/plain", "Saved. Rebooting...");
    delay(500);
    ESP.restart();
  });
}
