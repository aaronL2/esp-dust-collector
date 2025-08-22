#include "comms.h"
#include "servo_control.h"
#include <ArduinoJson.h>
#include <config_ui.h>
#include <cstring>
#include <cstdio>
#include "version.h"

const uint8_t broadcastMac[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
CommsClass comms;

// Set when an acknowledgment packet is received from the base
static volatile bool registerAck = false;
static String pendingToken;

void CommsClass::begin() {
  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init failed!");
    return;
  }
  esp_now_register_recv_cb(onReceive);
  bool unset = true;
  for (int i = 0; i < 6; ++i) {
    if (baseMac[i] != 0) {
      unset = false;
      break;
    }
  }
  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, unset ? broadcastMac : baseMac, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  esp_err_t addStatus = esp_now_add_peer(&peerInfo);
  if (addStatus != ESP_OK) {
    Serial.printf("ESP-NOW: add_peer failed (%d), retrying\n", addStatus);
    esp_now_del_peer(peerInfo.peer_addr);
    addStatus = esp_now_add_peer(&peerInfo);
    if (addStatus != ESP_OK) {
      Serial.printf("ESP-NOW: add_peer retry failed (%d)\n", addStatus);
    }
  }
}

void CommsClass::setBaseMac(const String &macStr) { parseMac(macStr, baseMac); }

void CommsClass::parseMac(const String &macStr, uint8_t *mac) {
  sscanf(macStr.c_str(), "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx", &mac[0], &mac[1],
         &mac[2], &mac[3], &mac[4], &mac[5]);
}

void CommsClass::sendCurrent(float amps) {
  // Build a JSON packet so the base can easily parse the current reading
  JsonDocument doc;
  doc["type"] = "current";
  doc["mac"] = WiFi.macAddress();
  doc["amps"] = amps;

  uint8_t buf[64];
  size_t len = serializeJson(doc, buf);
  sendToBase(buf, len);
}

void CommsClass::onReceive(const uint8_t *mac, const uint8_t *data, int len) {
  bool unset = true;
  for (int i = 0; i < 6; ++i) {
    if (comms.baseMac[i] != 0) {
      unset = false;
      break;
    }
  }
  if (unset) {
    Serial.println("ESP-NOW: base MAC unset, adopting sender");
    memcpy(comms.baseMac, mac, 6);
    char buf[18];
    snprintf(buf, sizeof(buf), "%02X:%02X:%02X:%02X:%02X:%02X",
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    configUI.setBaseMac(String(buf));
    configUI.saveConfig();
  
    esp_now_peer_info_t peerInfo = {};
    memcpy(peerInfo.peer_addr, mac, 6);
    peerInfo.channel = 0;
    peerInfo.encrypt = false;
    esp_err_t addStatus = esp_now_add_peer(&peerInfo);
    if (addStatus != ESP_OK) {
      Serial.printf("ESP-NOW: add_peer failed (%d)\n", addStatus);
    }
}

  // Ignore packets not originating from the base
  if (memcmp(mac, comms.baseMac, 6) != 0) {
    return;
  }

  if (len == 1 && (data[0] == 0 || data[0] == 1)) {
    if (data[0] == 1) {
      ServoControl.moveTo(90);  // open gate
    } else {
      ServoControl.moveTo(0);   // close gate
    }
    return;
  }

  // Parse the packet as JSON and check for a matching acknowledgment token
  JsonDocument doc;
  if (deserializeJson(doc, data, len) != DeserializationError::Ok) {
    return;
  }

  String type = doc["type"] | "";
  String token = doc["token"] | "";
  if (type == "ack" && token == pendingToken) {
    registerAck = true;
  }
}


bool registerWithBaseNow() {
  JsonDocument doc;
  doc["type"] = "register";
  doc["name"] = configUI.getFriendlyName();
  doc["mac"] = WiFi.macAddress();
  doc["version"] = Version::firmware();
  doc["fw"] = Version::firmware();
  doc["timestamp"] = String(millis() / 1000);
  String token = String(micros());
  doc["token"] = token;
  pendingToken = token;

  uint8_t buf[256];
  serializeJson(doc, Serial);
  Serial.println(); // DEBUG
  size_t len = serializeJson(doc, buf);

  registerAck = false;
  const int maxAttempts = 5;
  unsigned long waitMs = 100;

  for (int attempt = 0; attempt < maxAttempts; ++attempt) {
    comms.sendToBase(buf, len);

    unsigned long start = millis();
    while (!registerAck && millis() - start < waitMs) {
      delay(10);
    }

    if (registerAck) {
      return true;
    }

    waitMs *= 2;  // exponential backoff
  }

  return false;
}

void CommsClass::sendToBase(const uint8_t *data, size_t len) {
  bool unset = true;
  for (int i = 0; i < 6; ++i) {
    if (baseMac[i] != 0) {
      unset = false;
      break;
    }
  }
  const uint8_t *dest = unset ? broadcastMac : baseMac;

  if (esp_now_send(dest, data, len) == ESP_OK) {
    Serial.println(unset ? "ESP-NOW: broadcast packet" :
                             "ESP-NOW: sent packet to base");
  } else {
    Serial.println("ESP-NOW: failed to send packet");
  }
}
