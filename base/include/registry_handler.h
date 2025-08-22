#pragma once
#include <Arduino.h>
#include <ESPAsyncWebServer.h>

void setupRegistryRoutes(AsyncWebServer& server);
void updateStationRegistry(const String& mac, const String& name,
                           const String& fw = "",
                           const String& timestamp = "");