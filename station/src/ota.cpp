#include <ElegantOTA.h>
#include "ota.h"

void setupOTA(AsyncWebServer& server) {
    ElegantOTA.begin(&server);  // Enable OTA on /update
    Serial.println("ElegantOTA OTA ready at /update");
}
