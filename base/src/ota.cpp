#include "ota.h"
#include <AsyncElegantOTA.h>

void setupOTA(AsyncWebServer& server) {
    AsyncElegantOTA.begin(&server);
    Serial.println("OTA Ready. Visit /update to upload new firmware.");
}