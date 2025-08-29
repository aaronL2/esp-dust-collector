#pragma once
#include <Arduino.h>
class AsyncWebServer;

class ConfigUI {
public:
  virtual ~ConfigUI() = default;
  virtual void begin(AsyncWebServer& server) = 0;
  virtual void loadConfig() = 0;
  virtual void saveConfig() = 0;
  virtual void setBaseMac(const String& mac) = 0;
  virtual String getFriendlyName() const = 0;
  virtual String getBaseMac() const = 0;
  virtual String getMdnsName() const = 0;

  // Allow implementations to expose a current activation threshold
  // used by the base to decide when to enable the dust collector relay.
  // Default implementations provide a disabled threshold of 0.0f so
  // existing code that doesn't care about this value can ignore it.
  virtual float getCurrentThreshold() const { return 0.0f; }
  virtual void setCurrentThreshold(float) {}
};

extern ConfigUI& configUI;

// Tracks whether mDNS was started successfully for redirect logic
extern bool mdnsStarted;
