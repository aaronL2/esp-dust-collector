#include "comms.h"
#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>
#include <FS.h>
#include <SPIFFS.h>

// External function from config_ui.cpp
extern void updateStationRegistry(const String& mac, const String& name, const String& version = "", const String& timestamp = "");

void onDataRecv(const uint8_t* mac, const uint8_t* incomingData, int len) {
  //StaticJsonDocument<256> doc;
  JsonDocument doc;
  //doc.reserve(256);
  DeserializationError err = deserializeJson(doc, incomingData, len);
  if (err) {
    Serial.println("Failed to parse ESP-NOW JSON");
    return;
  }

  if (doc["type"] == "register") {
    String macStr = doc["mac"];
    String name = doc["name"];
    String version = doc["version"] | "";
    String timestamp = doc["timestamp"] | "";
    updateStationRegistry(macStr, name, version, timestamp);
    Serial.printf("ESP-NOW Register: %s (%s)\n", name.c_str(), macStr.c_str());
  } else if (doc["type"] == "ping") {
    String macStr = doc["mac"];
    updateStationRegistry(macStr, "", "", "");
    Serial.printf("ESP-NOW Ping: %s\n", macStr.c_str());
  }
}

void comms_setup() {
  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  esp_now_register_recv_cb(onDataRecv);
  Serial.println("ESP-NOW communication initialized (Base)");
}
