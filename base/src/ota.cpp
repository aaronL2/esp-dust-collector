#include "ota.h"

void setupOTA(AsyncWebServer& server) {
  ElegantOTA.begin(&server);   // /update auto-registered
  Serial.println("ElegantOTA ready at /update");
}
