#pragma once
#include <Arduino.h>
class AsyncWebServer;

class ConfigUI {
public:
  virtual ~ConfigUI() = default;
  virtual void begin(AsyncWebServer& server) = 0;
  virtual void loadConfig() = 0;
  virtual void saveConfig() = 0;
  virtual String getFriendlyName() const = 0;
  virtual String getBaseMac() const = 0;
  virtual String getMdnsName() const = 0;
};

extern ConfigUI& configUI;

