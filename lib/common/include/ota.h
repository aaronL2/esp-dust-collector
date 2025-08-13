#pragma once
#include <ESPAsyncWebServer.h>

// Ensure ElegantOTA uses AsyncWebServer integration
#ifndef ELEGANTOTA_USE_ASYNC_WEBSERVER
#define ELEGANTOTA_USE_ASYNC_WEBSERVER 1
#endif
#include <ElegantOTA.h>

void setupOTA(AsyncWebServer& server);
