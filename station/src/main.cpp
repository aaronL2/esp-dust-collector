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
#include "config_ui.h"
#include "current_sensor.h"
#include "ota.h"
#include "servo_control.h"
#include "version.h"
#include "wifi_manager.h"

extern CommsClass comms;
extern Display display;

static DisplayStatus status(display.getU8g2());

static unsigned long lastOledUpdate = 0;
static const unsigned long OLED_UPDATE_MS = 10000;

static void updateOled() {
  const String name = getFriendlyName();
  const String mdns = String(getMdnsName()) + ".local";
  const String ip   = WiFi.isConnected() ? WiFi.localIP().toString() : "-";
  const String mac  = WiFi.macAddress();
  const String fw   = String(FW_VERSION);
  status.show(name, mdns, ip, mac, fw);
}

void setup() {
  Serial.begin(115200);
  delay(100);
  
  setupWiFi(); 
  
  Serial.println();
  Serial.println("=============================");
  Serial.printf("Starting Station: %s\n", getFriendlyName().c_str());
  Serial.println("=============================");
  
  ServoControl.begin();
  CurrentSensor.begin();
  
  Serial.printf("\n✅ Station Wi-Fi connected. IP: %s\n", WiFi.localIP().toString().c_str());
  display.begin();
  status.begin();
  updateOled();

  configUI_setup();
  comms.setBaseMac(getBaseMac());
  comms.begin();
}

void loop() {
  // Read and print current
  float current = CurrentSensor.read();
  Serial.printf("Current: %.2f A\n", current);
  ElegantOTA.loop();   // <- required so OTA can trigger reboot

  if (millis() - lastOledUpdate >= OLED_UPDATE_MS) {
   lastOledUpdate = millis();
   updateOled();
  }

  delay(1000); // optional throttle
}
