#ifndef COMMS_H
#define COMMS_H

#include <esp_now.h>
#include <WiFi.h>

class CommsClass {
public:
  void begin();
  void setBaseMac(const String& macStr);
  void sendCurrent(float amps);

private:
  uint8_t baseMac[6];
  void parseMac(const String& macStr, uint8_t* mac);
  static void onReceive(const uint8_t* mac, const uint8_t* data, int len);
};

extern CommsClass Comms;

#endif
