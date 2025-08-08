
#ifndef BASE_CONFIG_UI_H
#define BASE_CONFIG_UI_H

#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>

class BaseConfigUI {
  public:
    void begin(AsyncWebServer &server);
    void loadConfig();
    void saveConfig();
    void setWifiSSID(const String& ssid);
    void setWifiPassword(const String& password);
    String getWifiSSID();
    String getWifiPassword();
    String getFriendlyName();
    String getBaseMac();

  private:
    DynamicJsonDocument config = DynamicJsonDocument(1024);
};

extern BaseConfigUI configUI;

#endif
