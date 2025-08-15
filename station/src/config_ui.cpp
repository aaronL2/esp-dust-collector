// src/config_ui.cpp  (STATION)

#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <FS.h>
#include <SPIFFS.h>
#include <ESPmDNS.h>
#include <WiFi.h>
#include <HTTPClient.h>

#include <Arduino.h>

#include "esp_timer.h"
#include "config_ui.h"
#include "ota.h"
#include "version.h"

// ---------------- Station state ----------------
AsyncWebServer server(80);

static String friendlyName = "station";
static String baseMac = "";   // reserved for future use if you pair via MAC

// ---------------- Persistence ----------------
static void loadConfig() {
  if (!SPIFFS.begin(true)) {
    Serial.println("SPIFFS mount failed");
    return;
  }
  File f = SPIFFS.open("/config.json", "r");
  if (!f) return;

  JsonDocument doc;
  if (deserializeJson(doc, f) == DeserializationError::Ok) {
    if (doc["friendlyName"].is<String>()) {
      friendlyName = doc["friendlyName"].as<String>();   // <- assign
    }
    if (doc["baseMac"].is<String>()) {
      baseMac = doc["baseMac"].as<String>();             // <- assign
    }
  }
  f.close();
}

static void saveConfig() {
  File f = SPIFFS.open("/config.json", FILE_WRITE);
  if (!f) return;
  JsonDocument doc;
  doc["friendlyName"] = friendlyName;
  doc["baseMac"]      = baseMac;
  serializeJson(doc, f);
  f.close();
}

// ---------------- Web UI / API ----------------
void configUI_setup() {
  loadConfig();

  // mDNS: http://<friendlyNameWithoutSpaces>.local
  if (MDNS.begin(getMdnsName().c_str())) {
    Serial.printf("mDNS: http://%s.local\n", getMdnsName().c_str());
  } else {
    Serial.println("mDNS start failed");
  }

  // Serve UI
  server.on("/", HTTP_GET, [](AsyncWebServerRequest* req) {
    req->send(SPIFFS, "/index.html", "text/html");
  });

  // Station status (preferred for UI because it includes FW)
  // { name, mac, ip, mdns, fw }
  server.on("/status", HTTP_GET, [](AsyncWebServerRequest* req) {
    JsonDocument doc;
    doc["name"] = friendlyName;
    doc["mac"]  = WiFi.macAddress();
    doc["ip"]   = WiFi.localIP().toString();
    doc["mdns"] = getMdnsName() + ".local";
    doc["fw"]   = Version::firmware();
    String out; serializeJson(doc, out);
    req->send(200, "application/json", out);
  });

  // (Legacy) /info kept for backward compatibility — mirrors /status minus FW
  server.on("/info", HTTP_GET, [](AsyncWebServerRequest* req) {
    JsonDocument doc;
    doc["name"] = friendlyName;
    doc["mac"]  = WiFi.macAddress();
    doc["ip"]   = WiFi.localIP().toString();
    doc["mdns"] = getMdnsName() + ".local";
    String out; serializeJson(doc, out);
    req->send(200, "application/json", out);
  });

  // Rename station (JSON: { "name": "<new>" }) -> responds with { newMdns }
  // Then triggers a clean reboot shortly after responding.
  server.on("/set-name", HTTP_POST,
  [](AsyncWebServerRequest* /*req*/){},
  nullptr,
  [](AsyncWebServerRequest* req, uint8_t* data, size_t len, size_t, size_t) {
    JsonDocument body;
    DeserializationError err = deserializeJson(body, data, len);

    // Invalid if deserialization failed OR missing/empty "name"
    if (err || body["name"].isNull() || !body["name"].is<const char*>()
        || String((const char*)body["name"]).isEmpty()) {
      req->send(400, "text/plain", "Invalid name");
      return;
    }

    friendlyName = body["name"].as<String>();
    saveConfig();

    JsonDocument res;
    res["newMdns"] = "http://" + getMdnsName() + ".local";
    String out; serializeJson(res, out);
    req->send(200, "application/json", out);

    // Non-blocking reboot ~500ms later
    esp_timer_handle_t reboot_timer;
    esp_timer_create_args_t args = {};
    args.callback = [](void*) { ESP.restart(); };
    args.name = "rebootTimer";
    if (esp_timer_create(&args, &reboot_timer) == ESP_OK) {
      esp_timer_start_once(reboot_timer, 500000); // microseconds
    }
  }
);

  // Register with Base over HTTP (Station -> Base)
  // POST to http://base.local/register  body: { name, mac, ip, mdns, fw }
  server.on("/register-with-base", HTTP_POST, [](AsyncWebServerRequest* req) {
    const String targetURL = "http://base.local/register";

    JsonDocument doc;
    doc["name"] = friendlyName;
    doc["mac"]  = WiFi.macAddress();
    doc["ip"]   = WiFi.localIP().toString();
    doc["mdns"] = getMdnsName() + ".local";
    doc["fw"]   = Version::firmware();

    String json; serializeJson(doc, json);
    Serial.printf("Registering with Base: %s\n", json.c_str());

    WiFiClient client;
    HTTPClient http;
    http.begin(client, targetURL);
    http.addHeader("Content-Type", "application/json");
    const int httpCode = http.POST(json);

    String response = "Registration ";
    if (httpCode > 0) {
      response += "succeeded: ";
      response += http.getString();
    } else {
      response += "failed: ";
      response += http.errorToString(httpCode);
    }
    http.end();

    req->send(200, "text/plain", response);
  });

  // OTA (ElegantOTA or your wrapper)
  setupOTA(server);

  server.begin();
}

// mDNS name helper: strip spaces
String getMdnsName() {
  String clean = friendlyName;
  clean.replace(" ", "");
  return clean;
}

String getFriendlyName() { return friendlyName; }
String getBaseMac()      { return baseMac; }
