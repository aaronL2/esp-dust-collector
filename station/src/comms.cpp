#include "comms.h"
#include "servo_control.h"
#include <ArduinoJson.h>
#include <config_ui.h>

CommsClass comms;

// Set when an acknowledgment packet is received from the base
static volatile bool registerAck = false;

void CommsClass::begin() {
  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init failed!");
    return;
  }
  esp_now_register_recv_cb(onReceive);

  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, baseMac, 6);
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

void CommsClass::setBaseMac(const String& macStr) {
  parseMac(macStr, baseMac);
}

void CommsClass::parseMac(const String& macStr, uint8_t* mac) {
  sscanf(macStr.c_str(), "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
         &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]);
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

void CommsClass::onReceive(const uint8_t* mac, const uint8_t* data, int len) {
  if (len == 1 && (data[0] == 0 || data[0] == 1)) {
    if (data[0] == 1) {
      ServoControl.moveTo(90);  // open gate
    } else {
      ServoControl.moveTo(0);   // close gate
    }
  } else {
    // Any non-servo packet is treated as an acknowledgment
    registerAck = true;
  }
}

bool registerWithBaseNow() {
  JsonDocument doc;
  doc["type"] = "register";
  doc["name"] = configUI.getFriendlyName();
  doc["mac"] = WiFi.macAddress();
  doc["version"] = "1.0.0";  // change to match your firmware version
  doc["timestamp"] = String(millis() / 1000);

  uint8_t buf[256];
  serializeJson(doc, Serial);
  Serial.println();  // DEBUG
  size_t len = serializeJson(doc, buf);

  registerAck = false;
  comms.sendToBase(buf, len);

  unsigned long start = millis();
  while (!registerAck && millis() - start < 1000) {
    delay(10);
  }
  return registerAck;
}

void CommsClass::sendToBase(const uint8_t* data, size_t len) {
  if (esp_now_send(baseMac, data, len) == ESP_OK) {
    Serial.println("ESP-NOW: sent registration to base");
  } else {
    Serial.println("ESP-NOW: failed to send registration");
  }
}
