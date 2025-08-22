#include "comms.h"
#include "registry_handler.h"
#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>
#include <FS.h>
#include <SPIFFS.h>
#include <esp_err.h>

static String macToString(const uint8_t* mac) {
  char buf[18];
  snprintf(buf, sizeof(buf), "%02X:%02X:%02X:%02X:%02X:%02X",
           mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  return String(buf);
}

void onDataRecv(const uint8_t* mac, const uint8_t* incomingData, int len) {
  //StaticJsonDocument<256> doc;
  JsonDocument doc;
  //doc.reserve(256);
  DeserializationError err = deserializeJson(doc, incomingData, len);
  if (err) {
    Serial.println("Failed to parse ESP-NOW JSON");
    return;
  }

  String jsonMac = doc["mac"] | "";
  String senderMac = macToString(mac);
  String macStr = jsonMac.isEmpty() ? senderMac : jsonMac;
  if (!jsonMac.isEmpty() && jsonMac != senderMac) {
    Serial.printf("ESP-NOW: sender MAC %s doesn't match payload %s\n",
                  senderMac.c_str(), macStr.c_str());
    return;
  }

  if (doc["type"] == "register") {
    serializeJson(doc, Serial);
    Serial.println();
    String name = doc["name"];
    String fw = doc["fw"] | "";
    if (fw.isEmpty()) fw = doc["version"] | "";
    String timestamp = doc["timestamp"] | "";
    String token = doc["token"] | "";
    updateStationRegistry(macStr, name, fw, timestamp);
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

    // Send a JSON acknowledgment back to the registering station echoing the token
    JsonDocument ackDoc;
    ackDoc["type"] = "ack";
    ackDoc["token"] = token;
    uint8_t ackBuf[64];
    size_t ackLen = serializeJson(ackDoc, ackBuf);
    esp_err_t sendStatus = esp_now_send(mac, ackBuf, ackLen);
    if (sendStatus == ESP_OK) {
      Serial.println("ESP-NOW: sent ack");
    } else {
      Serial.printf("ESP-NOW: failed to send ack (%d)\n", sendStatus);
    }
  } else if (doc["type"] == "ping") {
    updateStationRegistry(macStr, "", "", "");
    Serial.printf("ESP-NOW Ping: %s\n", macStr.c_str());
  } else if (doc["type"] == "current") {
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
