#include <ElegantOTA_Async.h>
#include "ota.h"

void setupOTA(AsyncWebServer& server) {
  ElegantOTA.begin(&server);  // OTA update page at /update
}
