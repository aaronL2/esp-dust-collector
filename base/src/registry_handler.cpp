#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <FS.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>
#include <esp_now.h>
#include <esp_err.h>
#include <cstdio>

void updateStationRegistry(const String& mac, const String& name,
                           const String& fw, const String& timestamp) {
  JsonDocument doc;
  File file = SPIFFS.open("/registry.json", "r");
  if (file) {
    deserializeJson(doc, file);
    file.close();
  }

  JsonArray arr = doc.to<JsonArray>();
  JsonObject target;
  for (JsonObject obj : arr) {
    if (mac == obj["mac"].as<String>()) {
      target = obj;
      break;
    }
  }
  if (!target) {
    target = arr.add<JsonObject>();
  }

  target["mac"] = mac;
  if (!name.isEmpty()) {
    target["name"] = name;
  } else if (!target.containsKey("name")) {
    target["name"] = "";
  }

  if (!fw.isEmpty()) {
    target["fw"] = fw;
    target["version"] = fw;
  } else {
    if (!target.containsKey("fw")) {
      target["fw"] = "";
    }
    if (!target.containsKey("version")) {
      target["version"] = "";
    }
  }

  if (!timestamp.isEmpty()) {
    target["timestamp"] = timestamp;
  } else {
    target["timestamp"] = String(millis());
  }

  file = SPIFFS.open("/registry.json", "w");
  if (file) {
    serializeJson(doc, file);
    String debug;
    serializeJson(doc, debug);
    Serial.printf("Registry updated: %s\n", debug.c_str());
    file.close();
  }
}

void setupRegistryRoutes(AsyncWebServer& server) {
  server.on("/registry.json", HTTP_GET, [](AsyncWebServerRequest *request) {
    File file = SPIFFS.open("/registry.json", "r");
    if (!file || file.size() == 0) {
      request->send(200, "application/json", "[]");
      return;
    }
    request->send(SPIFFS, "/registry.json", "application/json");
  });

  server.on("/registry", HTTP_POST, [](AsyncWebServerRequest *request) {}, nullptr,
  [](AsyncWebServerRequest* request, uint8_t* data, size_t len, size_t, size_t) {
    JsonDocument incoming;
    DeserializationError err = deserializeJson(incoming, data, len);
    if (err) {
      JsonDocument doc;
      doc["error"] = "Invalid JSON";
      String out; serializeJson(doc, out);
      request->send(400, "application/json", out);
      return;
    }

    const String newName = incoming["name"] | "";
    const String newMac  = incoming["mac"]  | "";
    String newFw = incoming["fw"] | "";
    if (newFw.isEmpty()) newFw = incoming["version"] | "";
    if (newName.isEmpty() || newMac.isEmpty()) {
      JsonDocument doc;
      doc["error"] = "Missing name or mac";
      String out; serializeJson(doc, out);
      request->send(400, "application/json", out);
      return;
    }

    JsonDocument doc;      // load registry (array of entries)
    File file = SPIFFS.open("/registry.json", "r");
    if (file) {
      deserializeJson(doc, file);
      file.close();
    }

    // Ensure root is an array (AJv7: use to<JsonArray>() to coerce)
    JsonArray arr = doc.to<JsonArray>();

    bool found = false;
    for (JsonObject obj : arr) {
      if (newMac == obj["mac"].as<String>()) {
        obj["name"] = newName;
        if (!newFw.isEmpty()) obj["fw"] = newFw;
        found = true;
        break;
      }
    }

    if (!found) {
      JsonObject entry = arr.add<JsonObject>();
      entry["name"] = newName;     // <- fixed typo: was newObj
      entry["mac"]  = newMac;
      if (!newFw.isEmpty()) entry["fw"] = newFw;
    }

    file = SPIFFS.open("/registry.json", "w");
    serializeJson(doc, file);
    file.close();

    JsonDocument resp;
    resp["success"] = true;
    String out; serializeJson(resp, out);
    request->send(200, "application/json", out);
  } 
  );

  server.on("/registry", HTTP_DELETE, [](AsyncWebServerRequest *request) {
    if (!request->hasParam("mac")) {
      JsonDocument resp;
      resp["error"] = "Missing mac";
      String out; serializeJson(resp, out);
      request->send(400, "application/json", out);
      return;
    }

    String mac = request->getParam("mac")->value();

    JsonDocument doc;
    File file = SPIFFS.open("/registry.json", "r");
    if (file) {
      deserializeJson(doc, file);
      file.close();
    }

    JsonArray arr = doc.to<JsonArray>();
    bool removed = false;
    for (JsonArray::iterator it = arr.begin(); it != arr.end(); ++it) {
      JsonObject obj = *it;
      if (mac == obj["mac"].as<String>()) {
        arr.remove(it);
        removed = true;
        break;
      }
    }

    if (removed) {
      uint8_t macBytes[6];
      sscanf(mac.c_str(), "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
             &macBytes[0], &macBytes[1], &macBytes[2],
             &macBytes[3], &macBytes[4], &macBytes[5]);

      JsonDocument msg;
      msg["type"] = "unregister";
      uint8_t buf[32];
      size_t len = serializeJson(msg, buf);
      esp_err_t status = esp_now_send(macBytes, buf, len);
      if (status != ESP_OK) {
        Serial.printf("ESP-NOW: failed to send unregister (%d)\n", status);
      }
    }

    file = SPIFFS.open("/registry.json", "w");
    serializeJson(doc, file);
    file.close();

    JsonDocument resp;
    resp["success"] = true;
    String out; serializeJson(resp, out);
    request->send(200, "application/json", out);
  });
}
