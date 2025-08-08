#ifndef OTA_H
#define OTA_H

#include <ESPAsyncWebServer.h>
#ifndef ELEGANTOTA_USE_ASYNC_WEBSERVER
#define ELEGANTOTA_USE_ASYNC_WEBSERVER 1
#endif
#include <ElegantOTA.h>

void setupOTA(AsyncWebServer& server);

#endif
