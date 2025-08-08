
#include <Arduino.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <SPIFFS.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <DNSServer.h>

#include "base_config_ui.h"
#include "registry_handler.h"
#include "display.h"
#include "ota.h"

AsyncWebServer server(80);
DNSServer dnsServer;
Display display;

void startConfigPortal() {
  WiFi.mode(WIFI_AP);
  const char* ap_ssid = "DustCollector_Base";
  WiFi.softAP(ap_ssid);
  IPAddress ip = WiFi.softAPIP();
  Serial.printf("📶 AP started: %s  IP: %s\n", ap_ssid, ip.toString().c_str());

  if (!SPIFFS.begin(true)) {
    Serial.println("❌ Failed to mount SPIFFS (portal)");
  }

  dnsServer.start(53, "*", ip);

  const char* formHtml =
    "<!doctype html><html><head><meta name='viewport' content='width=device-width,initial-scale=1'>"
    "<title>WiFi Setup</title></head><body>"
    "<h2>Dust Collector Wi‑Fi Setup</h2>"
    "<form method='POST' action='/save'>"
    "SSID: <input name='ssid'><br>"
    "Password: <input name='pass' type='password'><br><br>"
    "<button type='submit'>Save & Reboot</button>"
    "</form></body></html>";

  server.reset();
  server.onNotFound([formHtml](AsyncWebServerRequest* req) {
    req->send(200, "text/html", formHtml);
  });

  server.on("/save", HTTP_POST, [](AsyncWebServerRequest* req) {
    String ssid, pass;
    if (req->hasParam("ssid", true)) ssid = req->getParam("ssid", true)->value();
    if (req->hasParam("pass", true)) pass = req->getParam("pass", true)->value();
    if (ssid.length() == 0) {
      req->send(400, "text/plain", "SSID required");
      return;
    }
    configUI.setWifiSSID(ssid);
    configUI.setWifiPassword(pass);
    configUI.saveConfig();
    req->send(200, "text/plain", "Saved. Rebooting...");
    delay(750);
    ESP.restart();
  });

  server.begin();
  Serial.println("✅ Captive portal started (visit any URL)");
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\nBooting Base...");

  configUI.loadConfig();

  Serial.println("Connecting to Wi-Fi...");
  WiFi.mode(WIFI_STA);
  WiFi.begin(configUI.getWifiSSID(), configUI.getWifiPassword());

  unsigned long startAttemptTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 10000) {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("\n❌ Failed to connect to Wi-Fi. Starting config portal...");
    startConfigPortal();
    return;
  }

  Serial.println("\n✅ Wi-Fi connected.");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  if (!SPIFFS.begin(true)) {
    Serial.println("❌ Failed to mount SPIFFS");
    return;
  }

  if (MDNS.begin(configUI.getFriendlyName())) {
    Serial.println(String("http://") + configUI.getFriendlyName() + ".local");
  } else {
    Serial.println("❌ Error setting up mDNS responder!");
  }

  display.begin();
  display.update(configUI.getFriendlyName(), WiFi.localIP().toString(), WiFi.macAddress());

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
  dnsServer.processNextRequest();
  display.loop();
}
