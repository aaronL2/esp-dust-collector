#include "comms.h"
#include "registry_handler.h"
#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>
#include <FS.h>
#include <SPIFFS.h>
#include <esp_err.h>

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

    // Ensure the sender is a known peer so we can respond
    if (!esp_now_is_peer_exist(mac)) {
      esp_now_peer_info_t peerInfo = {};
      memcpy(peerInfo.peer_addr, mac, 6);
      peerInfo.channel = 0;
      peerInfo.encrypt = false;
      esp_err_t addStatus = esp_now_add_peer(&peerInfo);
      if (addStatus != ESP_OK) {
        Serial.printf("ESP-NOW: add_peer failed (%d)\n", addStatus);
      }
    }

    // Send a simple acknowledgment back to the registering station
    const uint8_t ack[] = {'a', 'c', 'k'};
    esp_err_t sendStatus = esp_now_send(mac, ack, sizeof(ack));
    if (sendStatus == ESP_OK) {
      Serial.println("ESP-NOW: sent ack");
    } else {
      Serial.printf("ESP-NOW: failed to send ack (%d)\n", sendStatus);
    }
  } else if (doc["type"] == "ping") {
    String macStr = doc["mac"];
    updateStationRegistry(macStr, "", "", "");
    Serial.printf("ESP-NOW Ping: %s\n", macStr.c_str());
  } else if (doc["type"] == "current") {
    String macStr = doc["mac"] | "";
    float amps = doc["amps"] | 0.0f;
    updateStationRegistry(macStr, "", "", "");
    Serial.printf("ESP-NOW Current: %.2f A from %s\n", amps, macStr.c_str());
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