#include "base_config_ui.h"
#include <SPIFFS.h>
#include <FS.h>
#include <ArduinoJson.h>

namespace configUI {
  String friendlyName = "base";
  String wifiSSID = "Landry";
  String wifiPassword = "";

  void loadConfig() {
    if (!SPIFFS.begin(true)) {
      Serial.println("❌ Failed to mount SPIFFS");
      return;
    }

    File file = SPIFFS.open("/config.json", "r");
    if (!file) {
      Serial.println("⚠️ No config.json found, using defaults");
      return;
    }

    StaticJsonDocument<512> doc;
    DeserializationError error = deserializeJson(doc, file);
    if (error) {
      Serial.println("❌ Failed to parse config.json");
      return;
    }

    if (doc.containsKey("friendlyName"))
      friendlyName = doc["friendlyName"].as<String>();

    if (doc["wifi"].containsKey("ssid"))
      wifiSSID = doc["wifi"]["ssid"].as<String>();
    if (doc["wifi"].containsKey("password"))
      wifiPassword = doc["wifi"]["password"].as<String>();
  }

  String getFriendlyName() { return friendlyName; }
  String getWifiSSID() { return wifiSSID; }
  String getWifiPassword() { return wifiPassword; }

  void begin(AsyncWebServer& server) {
    // Web UI setup placeholder
    server.on("/config", HTTP_GET, [](AsyncWebServerRequest *request) {
      StaticJsonDocument<256> doc;
      doc["friendlyName"] = friendlyName;
      doc["wifi"]["ssid"] = wifiSSID;
      doc["wifi"]["password"] = wifiPassword;
      String json;
      serializeJson(doc, json);
      request->send(200, "application/json", json);
    });
  }
}
