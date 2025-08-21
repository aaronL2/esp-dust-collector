#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <FS.h>
#include <SPIFFS.h>
#include <ESPmDNS.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <esp_timer.h>

#include <config_ui.h>
#include "comms.h"
#include "ota.h"
#include "version.h"

class StationConfigUI : public ConfigUI {
public:
  void begin(AsyncWebServer& server) override {
    if (MDNS.begin(getMdnsName().c_str())) {
      Serial.printf("mDNS: http://%s.local\n", getMdnsName().c_str());
    } else {
      Serial.println("mDNS start failed");
    }

    server.on("/", HTTP_GET, [](AsyncWebServerRequest* req) {
      req->send(SPIFFS, "/index.html", "text/html");
    });

    server.on("/status", HTTP_GET, [](AsyncWebServerRequest* req) {
      JsonDocument doc;
      doc["name"] = configUI.getFriendlyName();
      doc["mac"]  = WiFi.macAddress();
      doc["ip"]   = WiFi.localIP().toString();
      doc["mdns"] = configUI.getMdnsName() + ".local";
      doc["fw"]   = Version::firmware();
      String out; serializeJson(doc, out);
      req->send(200, "application/json", out);
    });

    server.on("/info", HTTP_GET, [](AsyncWebServerRequest* req) {
      JsonDocument doc;
      doc["name"] = configUI.getFriendlyName();
      doc["mac"]  = WiFi.macAddress();
      doc["ip"]   = WiFi.localIP().toString();
      doc["mdns"] = configUI.getMdnsName() + ".local";
      String out; serializeJson(doc, out);
      req->send(200, "application/json", out);
    });

    server.on("/set-name", HTTP_POST,
      [](AsyncWebServerRequest* /*req*/){},
      nullptr,
      [](AsyncWebServerRequest* req, uint8_t* data, size_t len, size_t, size_t) {
        JsonDocument body;
        DeserializationError err = deserializeJson(body, data, len);
        if (err || body["name"].isNull() || !body["name"].is<const char*>() ||
            String((const char*)body["name"]).isEmpty()) {
          req->send(400, "text/plain", "Invalid name");
          return;
        }
        auto& impl = static_cast<StationConfigUI&>(configUI);
        impl.setFriendlyName(body["name"].as<String>());
        impl.saveConfig();

        JsonDocument res;
        res["newMdns"] = "http://" + impl.getMdnsName() + ".local";
        String out; serializeJson(res, out);
        req->send(200, "application/json", out);

        esp_timer_handle_t reboot_timer;
        esp_timer_create_args_t args = {};
        args.callback = [](void*) { ESP.restart(); };
        args.name = "rebootTimer";
        if (esp_timer_create(&args, &reboot_timer) == ESP_OK) {
          esp_timer_start_once(reboot_timer, 500000);
        }
      });

    server.on("/register-with-base", HTTP_POST, [](AsyncWebServerRequest* req) {
      bool ok = registerWithBaseNow();
      req->send(ok ? 200 : 500, "text/plain",
                ok ? "Registered" : "No acknowledgment from base");
                  });

    setupOTA(server);
  }

  void loadConfig() override {
    if (!SPIFFS.begin(true)) {
      Serial.println("SPIFFS mount failed");
      return;
    }
    File f = SPIFFS.open("/config.json", "r");
    if (!f) return;
    JsonDocument doc;
    if (deserializeJson(doc, f) == DeserializationError::Ok) {
      if (doc["friendlyName"].is<String>()) {
        friendlyName = doc["friendlyName"].as<String>();
      }
      if (doc["baseMac"].is<String>()) {
        baseMac = doc["baseMac"].as<String>();
      }
    }
    f.close();
  }

  void saveConfig() override {
    File f = SPIFFS.open("/config.json", FILE_WRITE);
    if (!f) return;
    JsonDocument doc;
    doc["friendlyName"] = friendlyName;
    doc["baseMac"]      = baseMac;
    serializeJson(doc, f);
    f.close();
  }

  String getFriendlyName() const override { return friendlyName; }
  String getBaseMac() const override { return baseMac; }
  String getMdnsName() const override {
    String clean = friendlyName;
    clean.replace(" ", "");
    return clean;
  }

  void setFriendlyName(const String& name) { friendlyName = name; }
  void setBaseMac(const String& mac) override { baseMac = mac; }

private:
  String friendlyName = "station";
  String baseMac = "";   // reserved for future use if you pair via MAC
};

static StationConfigUI instance;
ConfigUI& configUI = instance;

