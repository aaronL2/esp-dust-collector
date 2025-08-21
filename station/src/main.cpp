#include <WiFi.h>
#include <ESPmDNS.h>
#include <U8g2lib.h>
#include <ArduinoJson.h>
#include <esp_now.h>
#include <Arduino.h>
#include <DNSServer.h>
#include <ElegantOTA.h>
#include <ESPAsyncWebServer.h>
#include <FS.h>
#include <SPIFFS.h>

#include "display.h"
#include "display_status.h"
#include "comms.h"
#include <config_ui.h>
#include "current_sensor.h"
#include "ota.h"
#include "servo_control.h"
#include "version.h"
#include "wifi_manager.h"

extern CommsClass comms;
extern Display display;

AsyncWebServer server(80);
static DisplayStatus status(display.getU8g2());

static unsigned long lastOledUpdate = 0;
static const unsigned long OLED_UPDATE_MS = 10000;
static unsigned long lastCurrentSend = 0;
static const unsigned long CURRENT_SEND_MS = 1000;
static bool registered = false;
static unsigned long lastRegisterAttempt = 0;
static const unsigned long REGISTER_RETRY_MS = 10000;

static void updateOled() {
  const String name = configUI.getFriendlyName();
  const String mdns = configUI.getMdnsName() + ".local";
  const String ip   = WiFi.isConnected() ? WiFi.localIP().toString() : "-";
  const String mac  = WiFi.macAddress();
  const String fw   = String(FW_VERSION);
  status.show(name, mdns, ip, mac, fw);
}

void setup() {
  Serial.begin(115200);
  delay(100);
  display.begin();
  status.begin();

  setupWiFi();
  configUI.loadConfig();

  Serial.println();
  Serial.println("=============================");
  Serial.printf("Starting Station: %s\n", configUI.getFriendlyName().c_str());
  Serial.println("=============================");
  
  ServoControl.begin();
  CurrentSensor.begin();
  
  Serial.printf("\n✅ Station Wi-Fi connected. IP: %s\n", WiFi.localIP().toString().c_str());
  updateOled();

  configUI.begin(server);
  comms.setBaseMac(configUI.getBaseMac());
  comms.begin();
  registered = registerWithBaseNow();
  lastRegisterAttempt = millis();
  Serial.printf("Initial registration %s\n", registered ? "succeeded" : "failed");
server.begin();
}

void loop() {
  unsigned long now = millis();

  if (!registered && now - lastRegisterAttempt >= REGISTER_RETRY_MS) {
    registered = registerWithBaseNow();
    lastRegisterAttempt = now;
    Serial.printf("Register retry %s\n", registered ? "succeeded" : "failed");
  }

  // Read and print current
  float current = CurrentSensor.read();
  Serial.printf("Current: %.2f A\n", current);
    unsigned long now = millis();
  if (now - lastCurrentSend >= CURRENT_SEND_MS) {
    comms.sendCurrent(current);
    lastCurrentSend = now;
  }
  ElegantOTA.loop();   // <- required so OTA can trigger reboot

  if (millis() - lastOledUpdate >= OLED_UPDATE_MS) {
   lastOledUpdate = millis();
   updateOled();
  }

  delay(1000); // optional throttle
}
