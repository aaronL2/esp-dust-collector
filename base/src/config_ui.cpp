#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>
#include <config_ui.h>

class BaseConfigUI : public ConfigUI {
public:
  void begin(AsyncWebServer& /*server*/) override {
    // optional: register base-specific routes here later
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

  void setWifiSSID(const String& ssid) { config["wifi_ssid"] = ssid; }
  void setWifiPassword(const String& pass) { config["wifi_password"] = pass; }
  String getWifiSSID() const { return config["wifi_ssid"] | ""; }
  String getWifiPassword() const { return config["wifi_password"] | ""; }

private:
  JsonDocument config;  // holds {"wifi_ssid","wifi_password","name","base_mac",...}
};

static BaseConfigUI instance;
ConfigUI& configUI = instance;

