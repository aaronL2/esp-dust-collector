#include <Arduino.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <SPIFFS.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <WiFiManager.h>

#include "base_config_ui.h"
#include "registry_handler.h"
#include "display.h"
#include "ota.h"

AsyncWebServer server(80);
Display display;

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\nBooting Base...");

  // Load config before WiFi (needed later for friendlyName, etc.)
  configUI.loadConfig();

  // Use WiFiManager for captive portal setup
  WiFiManager wm;
  wm.autoConnect("DustCollector_Base");

  // Optionally configure timeout for portal (e.g. 3 minutes)
  wm.setConfigPortalTimeout(180);

  // Start captive portal if no Wi-Fi or can't connect
  if (!wm.autoConnect("DustCollector_Base")) {
    Serial.println("❌ Failed to connect or timed out. Restarting...");
    delay(3000);
    ESP.restart();
  }

  // At this point, WiFi is connected
  Serial.println("✅ Wi-Fi connected.");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  if (!SPIFFS.begin(true)) {
    Serial.println("❌ Failed to mount SPIFFS");
    return;
  }

  // Dump SPIFFS contents
  Serial.println("📁 Listing and dumping SPIFFS files:");
  File root = SPIFFS.open("/");
  File file = root.openNextFile();
  while (file) {
    Serial.printf(" - %s (%d bytes)\n", file.name(), file.size());
    while (file.available()) {
      Serial.write(file.read());
    }
    Serial.println("\n---");
    file = root.openNextFile();
  }

  // mDNS setup using friendly name from config
  if (MDNS.begin(configUI.getFriendlyName())) {
    Serial.println(String("mDNS responder started: http://") + configUI.getFriendlyName() + ".local");
  } else {
    Serial.println("❌ Error setting up mDNS responder!");
  }

  // Initialize display
  display.begin();
  display.update(configUI.getFriendlyName(), WiFi.localIP().toString(), WiFi.macAddress());

  // Set up UI and registry
  configUI.begin(server);
  setupRegistryRoutes(server);

  server.serveStatic("/", SPIFFS, "/").setDefaultFile("index.html");

  server.on("/info", HTTP_GET, [](AsyncWebServerRequest *request) {
    StaticJsonDocument<256> doc;
    doc["name"] = configUI.getFriendlyName();
    doc["ip"] = WiFi.localIP().toString();
    doc["mdns"] = String(configUI.getFriendlyName()) + ".local";
    doc["mac"] = WiFi.macAddress();

    String json;
    serializeJson(doc, json);
    request->send(200, "application/json", json);
  });

  setupOTA(server);
  server.begin();
  Serial.println("✅ HTTP server started");
}

void loop() {
  display.loop();
}
