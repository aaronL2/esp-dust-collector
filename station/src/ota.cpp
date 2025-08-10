#include <ElegantOTA.h>
#include "ota.h"

void setupOTA(AsyncWebServer& server) {
    ElegantOTA.begin(&server);  // Enable OTA on /update
    ElegantOTA.setAutoReboot(true); // Force reboot after OTA completes
    Serial.println("ElegantOTA OTA ready at /update");
}
