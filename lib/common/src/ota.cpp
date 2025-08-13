#include "ota.h"

void setupOTA(AsyncWebServer& server) {
  ElegantOTA.begin(&server);      // registers /update
  ElegantOTA.setAutoReboot(true); // reboot after OTA completes

  // Optional auth (set via build flags if you want it later)
  #if defined(OTA_USER) && defined(OTA_PASS)
    ElegantOTA.setAuth(OTA_USER, OTA_PASS);
  #endif

  Serial.println("ElegantOTA ready at /update");
}
