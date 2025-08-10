#include "ota.h"

void setupOTA(AsyncWebServer& server) {
  ElegantOTA.begin(&server);   // /update auto-registered
  ElegantOTA.setAutoReboot(true); // Force reboot after OTA completes
  Serial.println("ElegantOTA ready at /update");
}
