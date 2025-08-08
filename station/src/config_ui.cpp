#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <FS.h>
#include <SPIFFS.h>
#include <ESPmDNS.h>
#include "config_ui.h"
#include <HTTPClient.h>
#include "esp_timer.h"
#include "ota.h"

AsyncWebServer server(80);
String friendlyName = "station";
String baseMac = "";

void loadConfig() {
  if (SPIFFS.begin(true)) {
    File configFile = SPIFFS.open("/config.json");
    if (configFile) {
      ArduinoJson::StaticJsonDocument<256> doc;
      if (deserializeJson(doc, configFile) == DeserializationError::Ok) {
        if (doc.containsKey("friendlyName")) friendlyName = doc["friendlyName"].as<String>();
        if (doc.containsKey("baseMac")) baseMac = doc["baseMac"].as<String>();
      }
      configFile.close();
    }
  }
}

void saveConfig() {
  File configFile = SPIFFS.open("/config.json", FILE_WRITE);
  if (configFile) {
    ArduinoJson::StaticJsonDocument<256> doc;
    doc["friendlyName"] = friendlyName;
    doc["baseMac"] = baseMac;
    serializeJson(doc, configFile);
    configFile.close();
  }
}

String getFriendlyName() { return friendlyName; }
String getBaseMac() { return baseMac; }
String getMdnsName();  // Forward declaration

void configUI_setup() {
  loadConfig();
  if (MDNS.begin(getMdnsName().c_str())) {
    Serial.printf("mDNS started: http://%s.local", getMdnsName().c_str());
  } else {
    Serial.println("Error starting mDNS");
  }

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/index.html", "text/html");
  });

  server.on("/info", HTTP_GET, [](AsyncWebServerRequest *request) {
    ArduinoJson::StaticJsonDocument<256> doc;
    doc["name"] = getFriendlyName();
    doc["mac"] = WiFi.macAddress();
    doc["ip"] = WiFi.localIP().toString();
    doc["mdns"] = "http://" + getMdnsName() + ".local";
    String json;
    serializeJson(doc, json);
    request->send(200, "application/json", json);
  });

  server.on("/set-name", HTTP_POST, [](AsyncWebServerRequest *request){},
    NULL,
    [](AsyncWebServerRequest *request, uint8_t* data, size_t len, size_t, size_t) {
      ArduinoJson::StaticJsonDocument<128> doc;
      DeserializationError err = deserializeJson(doc, data);
      if (err || !doc.containsKey("name")) {
        request->send(400, "text/plain", "Invalid name");
        return;
      }
      friendlyName = doc["name"].as<String>();
      saveConfig();

      String newMdns = "http://" + getMdnsName() + ".local";
      ArduinoJson::StaticJsonDocument<128> response;
      response["newMdns"] = newMdns;
      String json;
      serializeJson(response, json);
      request->send(200, "application/json", json);

      // Non-blocking reboot using esp_timer (500ms)
      esp_timer_handle_t reboot_timer;
      const esp_timer_create_args_t reboot_args = {
        .callback = [](void*) { ESP.restart(); },
        .name = "rebootTimer"
      };
      esp_timer_create(&reboot_args, &reboot_timer);
      esp_timer_start_once(reboot_timer, 500000);  // 500ms in microseconds
    });

  server.on("/register-with-base", HTTP_POST, [](AsyncWebServerRequest *request) {
    String targetURL = "http://base.local/register";
    ArduinoJson::StaticJsonDocument<256> doc;
    doc["name"] = getFriendlyName();
    doc["mac"] = WiFi.macAddress();
    doc["version"] = "1.0.0";
    doc["timestamp"] = String(millis() / 1000);
    serializeJson(doc, Serial); Serial.println();

    WiFiClient client;
    HTTPClient http;
    http.begin(client, targetURL);
    http.addHeader("Content-Type", "application/json");

    String json;
    serializeJson(doc, json);
    int httpCode = http.POST(json);
    String response = "Registration ";

    if (httpCode > 0) {
      response += "succeeded: ";
      response += http.getString();
    } else {
      response += "failed: ";
      response += http.errorToString(httpCode);
    }

    http.end();
    request->send(200, "text/plain", response);
  });
  
  setupOTA(server);
  server.begin();
}

String getMdnsName() {
  String clean = friendlyName;
  clean.replace(" ", "");
  return clean;
}
