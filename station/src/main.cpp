#include "ota.h"
#include <WiFi.h>
#include <esp_now.h>
#include <ESPmDNS.h>
#include "config.h"
#include "config_ui.h"
#include "display.h"
#include "servo_control.h"
#include "current_sensor.h"
#include "comms.h"
#include <Arduino.h>

extern CommsClass comms;
extern Display display;

void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.println();
  Serial.println("=============================");
  Serial.printf("Starting Station: %s\n", getFriendlyName().c_str());
  Serial.println("=============================");

  display.begin();
  ServoControl.begin();
  CurrentSensor.begin();

  Serial.printf("Connecting to SSID: %s\n", WIFI_SSID);
  Serial.printf("Using password: %s\n", WIFI_PASSWORD);

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");

  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 10000) {
    delay(300);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi connected!");
  } else {
    Serial.println("\nWiFi failed to connect.");
  }

  String friendlyName = getFriendlyName();
  if (MDNS.begin(friendlyName.c_str())) {
    Serial.printf("mDNS started: http://%s.local\n", friendlyName.c_str());
  }

  configUI_setup();
  comms.setBaseMac(getBaseMac());
  comms.begin();
  display.showDeviceInfo(friendlyName);
}

void loop() {
  static unsigned long lastSwitch = 0;
  static bool showQR = false;

  // Read and print current
  float current = CurrentSensor.read();
  Serial.printf("Current: %.2f A\n", current);

  if (millis() - lastSwitch > 5000) {
    lastSwitch = millis();
    showQR = !showQR;

    String name = getFriendlyName();
    if (showQR) {
      display.showQRCode("http://" + name + ".local");
    } else {
      display.showDeviceInfo(name);
    }
  }

  delay(1000); // optional throttle
}
