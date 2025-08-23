#include <Arduino.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <SPIFFS.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <DNSServer.h>
#include <U8g2lib.h>

#include "display_status.h"
#include "version.h"
#include <config_ui.h>
#include "registry_handler.h"
#include "display.h"
#include "ota.h"
#include "wifi_manager.h"
#include "comms.h"

// Fallback so builds still succeed even if the pre-build script didn't run
#ifndef FW_VERSION
#define FW_VERSION "0.00000000.000000"
#endif

AsyncWebServer server(80);
DNSServer dnsServer;

static DisplayStatus status(display.getU8g2());

static unsigned long lastOledUpdate = 0;
static const unsigned long OLED_UPDATE_MS = 10000;

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
  delay(1000);
  display.begin();
  status.begin();
  setupWiFi();
  Serial.println("\nBooting Base...");

  configUI.loadConfig();

  Serial.println("\n✅ Wi-Fi connected.");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  if (!SPIFFS.begin(true)) {
    Serial.println("❌ Failed to mount SPIFFS");
    return;
  }

  if (MDNS.begin(configUI.getMdnsName().c_str())) {
    Serial.println(String("http://") + configUI.getMdnsName() + ".local");
    mdnsStarted = true;
  }
  
  comms_setup();

  updateOled();

  configUI.begin(server);
  setupRegistryRoutes(server);

  server.serveStatic("/", SPIFFS, "/").setDefaultFile("index.html");

  // Legacy info endpoint (kept for compatibility)
  server.on("/info", HTTP_GET, [](AsyncWebServerRequest *request) {
    //StaticJsonDocument<256> doc;
    JsonDocument doc;
    //doc.reserve(256);
    doc["name"] = configUI.getFriendlyName();
    doc["ip"] = WiFi.localIP().toString();
    doc["mdns"] = configUI.getMdnsName() + ".local";
    doc["mac"] = WiFi.macAddress();
    String json;
    serializeJson(doc, json);
    request->send(200, "application/json", json);
  });

  // Unified status endpoint to match Station schema
  server.on("/status", HTTP_GET, [](AsyncWebServerRequest *request) {
    //StaticJsonDocument<256> doc;
    JsonDocument doc;
    //doc.reserve(256);
    doc["name"]   = configUI.getFriendlyName();
    doc["fw"]     = FW_VERSION;
    doc["ip"]     = WiFi.localIP().toString();
    doc["mdns"]   = configUI.getMdnsName() + ".local";
    doc["mac"]    = WiFi.macAddress();
    doc["role"]   = "base";
    doc["uptime"] = (uint32_t)(millis() / 1000);
    String json;
    serializeJson(doc, json);
    request->send(200, "application/json", json);
  });

  setupOTA(server);
  server.begin();
  Serial.println("✅ HTTP server started");
}

void loop() {
  ElegantOTA.loop();   // <- required so OTA can trigger reboot

  if (millis() - lastOledUpdate >= OLED_UPDATE_MS) {
  lastOledUpdate = millis();
  updateOled();
}

}
