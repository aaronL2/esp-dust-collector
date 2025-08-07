#pragma once
#include <Arduino.h>

class Display {
public:
  void begin();
  void update(const String& name, const String& ip, const String& mac);
  void loop(); // call this in main loop
private:
  void showDeviceInfo();
  void showQRCode();
  String name, ip, mac;
  unsigned long lastSwitchTime = 0;
  bool showingInfo = true;
};
