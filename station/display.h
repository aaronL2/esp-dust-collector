#pragma once
#include <Arduino.h>

class Display {
public:
  void begin();
  void showDeviceInfo(const String& name);
  void showQRCode(const String& url);
};

extern Display display;
