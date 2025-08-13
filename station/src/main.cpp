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

static DNSServer dnsServer; // used only during AP portal
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
  const String fw   = Version::firmware(); // or String(FW_VERSION)
  status.show(name, mdns, ip, mac, fw);
}

static void startConfigPortal_Blocking() {
  WiFi.mode(WIFI_AP);
  const char* ap_ssid = "DustCollector_Station";
  WiFi.softAP(ap_ssid);

  // OLED init (AP mode)
  display.begin();
  status.begin();

  IPAddress ip = WiFi.softAPIP();
  String mac   = WiFi.softAPmacAddress();

  // Show AP status
  status.show("AP: DustCollector_Station", "-", ip.toString(), mac, Version::firmware());
  Serial.printf("Station AP started: %s  IP: %s\n", ap_ssid, ip.toString().c_str());

  // DNS hijack: all domains -> 192.168.4.1
  dnsServer.start(53, "*", ip);

  // Minimal portal using a temporary AsyncWebServer just for AP mode
  static AsyncWebServer apServer(80);

  const char* formHtml =
    "<!doctype html><html><head><meta name='viewport' content='width=device-width,initial-scale=1'>"
    "<title>Station WiFi Setup</title></head><body>"
    "<h2>Station – Wi-Fi Setup</h2>"
    "<form method='POST' action='/save'>"
    "SSID: <input name='ssid'><br>"
    "Password: <input name='pass' type='password'><br><br>"
    "<button type='submit'>Save & Reboot</button>"
    "</form></body></html>";

  apServer.onNotFound([formHtml](AsyncWebServerRequest* req) {
    req->send(200, "text/html", formHtml);
  });

  apServer.on("/save", HTTP_POST, [](AsyncWebServerRequest* req) {
    String ssid, pass;
    if (req->hasParam("ssid", true)) ssid = req->getParam("ssid", true)->value();
    if (req->hasParam("pass", true)) pass = req->getParam("pass", true)->value();

    if (ssid.isEmpty()) {
      req->send(400, "text/plain", "SSID required");
      return;
    }

    // Persist creds
    if (!SPIFFS.begin(true)) {
      req->send(500, "text/plain", "SPIFFS mount failed");
      return;
    }
    File f = SPIFFS.open("/wifi.json", FILE_WRITE);
    if (!f) {
      req->send(500, "text/plain", "open /wifi.json failed");
      return;
    }

    JsonDocument doc;              // ArduinoJson v7
    doc["ssid"] = ssid;
    doc["pass"] = pass;
    serializeJson(doc, f);
    f.close();

    req->send(200, "text/plain", "Saved. Rebooting...");
    Serial.println("Station Wi-Fi creds saved. Rebooting...");
    delay(500);
    ESP.restart();
  });

  apServer.begin();
  Serial.println("Station captive portal started (visit any URL)");

  // Block here to keep DNS + portal alive until reboot
  for (;;) {
    dnsServer.processNextRequest();
    delay(10);
  }
}

void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.println();
  Serial.println("=============================");
  Serial.printf("Starting Station: %s\n", getFriendlyName().c_str());
  Serial.println("=============================");

  ServoControl.begin();
  CurrentSensor.begin();

String ssid, pass;
if (SPIFFS.begin(true)) {
  if (File f = SPIFFS.open("/wifi.json", FILE_READ)) {
    JsonDocument doc;  // v7
    if (deserializeJson(doc, f) == DeserializationError::Ok) {
      ssid = doc["ssid"] | String("");
      pass = doc["pass"] | String("");
    }
    f.close();
  }
}

  Serial.println("Connecting to Wi-Fi...");
  WiFi.mode(WIFI_STA);
  if (ssid.length()) WiFi.begin(ssid.c_str(), pass.c_str());
  else WiFi.begin(); // use NVS creds if any

  unsigned long t0 = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - t0 < 10000) {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("\n❌ Station failed to connect. Starting captive portal...");
    startConfigPortal_Blocking();
    return; // never reached (blocking until reboot)
  }

  Serial.printf("\n✅ Station Wi-Fi connected. IP: %s\n", WiFi.localIP().toString().c_str());
  display.begin();
  status.begin();
  updateOled();

  configUI_setup();
  comms.setBaseMac(getBaseMac());
  comms.begin();
}

void loop() {
  static unsigned long lastSwitch = 0;
  static bool showQR = false;

  dnsServer.processNextRequest();

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
