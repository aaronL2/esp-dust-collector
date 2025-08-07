#include "ota.h"
#define ELEGANTOTA_USE_ASYNC_WEBSERVER 1
#include <ElegantOTA.h>

void setupOTA(AsyncWebServer& server) {
    ElegantOTA.begin(&server);  // ✅ now uses our wrapper class
    Serial.println("ElegantOTA ready at /update");
}
