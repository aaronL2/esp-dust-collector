#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>
#include <config_ui.h>

class BaseConfigUI : public ConfigUI {
public:
  void begin(AsyncWebServer& server) override {
    // Endpoint to update the current-activation threshold. Expects a JSON
    // body of the form {"threshold": <float>} and persists the value to
    // config.json so it can be tuned at runtime.
    server.on("/set-threshold", HTTP_POST,
      [](AsyncWebServerRequest* /*req*/){},
      nullptr,
      [](AsyncWebServerRequest* req, uint8_t* data, size_t len, size_t, size_t) {
        JsonDocument body;
        DeserializationError err = deserializeJson(body, data, len);
        if (err || !body["threshold"].is<float>()) {
          req->send(400, "text/plain", "Invalid threshold");
          return;
        }
        auto& self = static_cast<BaseConfigUI&>(configUI);
        self.setCurrentThreshold(body["threshold"].as<float>());
        self.saveConfig();
        req->send(200, "text/plain", "OK");
      });
  }

  void loadConfig() override {
    if (!SPIFFS.begin(true)) return;
    File file = SPIFFS.open("/config.json");
    if (!file) return;
    DeserializationError err = deserializeJson(config, file);
    if (err) Serial.println("Failed to load config");
    file.close();
  }

  void saveConfig() override {
    File file = SPIFFS.open("/config.json", FILE_WRITE);
    if (!file) return;
    serializeJson(config, file);
    file.close();
  }

  String getFriendlyName() const override {
    return config["name"] | "dustbase";
  }

  String getBaseMac() const override {
    return config["base_mac"] | "";
  }

  String getMdnsName() const override {
    String n = getFriendlyName();
    n.replace(" ", "");
    return n;
  }

  void setBaseMac(const String& mac) override { config["base_mac"] = mac; }

    float getCurrentThreshold() const override {
    return config["threshold"] | 5.0f; // default of 5 amps
  }
  void setCurrentThreshold(float t) override { config["threshold"] = t; }

  void setWifiSSID(const String& ssid) { config["wifi_ssid"] = ssid; }
  void setWifiPassword(const String& pass) { config["wifi_password"] = pass; }
  String getWifiSSID() const { return config["wifi_ssid"] | ""; }
  String getWifiPassword() const { return config["wifi_password"] | ""; }

private:
  JsonDocument config;  // holds {"wifi_ssid","wifi_password","name","base_mac",...}
};

static BaseConfigUI instance;
ConfigUI& configUI = instance;

