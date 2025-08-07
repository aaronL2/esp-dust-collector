#include <Arduino.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <SPIFFS.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>

#include "base_config_ui.h"
#include "registry_handler.h"
#include "display.h"

AsyncWebServer server(80);
Display display;

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\nBooting Base...");

  configUI::loadConfig();

  Serial.println("Connecting to Wi-Fi...");
  WiFi.begin(configUI::getWifiSSID().c_str(), configUI::getWifiPassword().c_str());

  unsigned long startAttemptTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 10000) {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("\n❌ Failed to connect to Wi-Fi.");
    return;
  }

  Serial.println("\n✅ Wi-Fi connected.");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin(configUI::getFriendlyName().c_str())) {
    Serial.println("✅ mDNS responder started: http://" + configUI::getFriendlyName() + ".local");
  } else {
    Serial.println("❌ Error setting up mDNS responder!");
  }

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

  // Initialize display
  display.begin();
  display.update(configUI::getFriendlyName(), WiFi.localIP().toString(), WiFi.macAddress());

  // Set up UI and registry
  configUI::begin(server);
  setupRegistryRoutes(server);

  server.serveStatic("/", SPIFFS, "/").setDefaultFile("index.html");

  server.on("/info", HTTP_GET, [](AsyncWebServerRequest *request) {
    StaticJsonDocument<256> doc;
    doc["name"] = configUI::getFriendlyName();
    doc["ip"] = WiFi.localIP().toString();
    doc["mdns"] = configUI::getFriendlyName() + ".local";
    doc["mac"] = WiFi.macAddress();

    String json;
    serializeJson(doc, json);
    request->send(200, "application/json", json);
  });

  server.begin();
  Serial.println("✅ HTTP server started");
}

void loop() {
  display.loop();
}
