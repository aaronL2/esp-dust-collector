#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <FS.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>

void setupRegistryRoutes(AsyncWebServer& server) {
  server.on("/registry.json", HTTP_GET, [](AsyncWebServerRequest *request) {
    File file = SPIFFS.open("/registry.json", "r");
    if (!file || file.size() == 0) {
      request->send(200, "application/json", "[]");
      return;
    }
    request->send(file, "application/json");
  });

  server.on("/registry", HTTP_POST, [](AsyncWebServerRequest *request) {}, nullptr,
    [](AsyncWebServerRequest* request, uint8_t* data, size_t len, size_t, size_t) {
      StaticJsonDocument<256> incoming;
      DeserializationError err = deserializeJson(incoming, data);
      if (err) {
        StaticJsonDocument<64> doc;
        doc["error"] = "Invalid JSON";
        String out;
        serializeJson(doc, out);
        request->send(400, "application/json", out);
        return;
      }

      String newName = incoming["name"] | "";
      String newMac = incoming["mac"] | "";
      if (newName == "" || newMac == "") {
        StaticJsonDocument<64> doc;
        doc["error"] = "Missing name or mac";
        String out;
        serializeJson(doc, out);
        request->send(400, "application/json", out);
        return;
      }

      DynamicJsonDocument doc(1024);
      File file = SPIFFS.open("/registry.json", "r");
      if (file) {
        deserializeJson(doc, file);
        file.close();
      }

      bool found = false;
      for (JsonObject obj : doc.as<JsonArray>()) {
        if (obj["mac"] == newMac) {
          obj["name"] = newName;
          found = true;
          break;
        }
      }

      if (!found) {
        JsonObject newObj = doc.createNestedObject();
        newObj["name"] = newName;
        newObj["mac"] = newMac;
      }

      file = SPIFFS.open("/registry.json", "w");
      serializeJson(doc, file);
      file.close();

      StaticJsonDocument<64> resp;
      resp["success"] = true;
      String out;
      serializeJson(resp, out);
      request->send(200, "application/json", out);
    }
  );
}
