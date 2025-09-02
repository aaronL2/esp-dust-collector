#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <FS.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>
#include <esp_now.h>
#include <esp_err.h>
#include <cstdio>
#include "comms.h"
#include <map>

static std::map<String, float> calibrationSessions;

void updateStationRegistry(const String& mac, const String& name,
                           const String& fw, const String& timestamp,
                           float offDelay, float threshold) {
  JsonDocument doc;
  File file = SPIFFS.open("/registry.json", "r");
  if (file) {
    deserializeJson(doc, file);
    file.close();
  }

  JsonArray arr = doc.is<JsonArray>() ? doc.as<JsonArray>() : doc.to<JsonArray>();
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

  if (offDelay >= 0) {
    target["off_delay"] = offDelay;
  } else if (!target.containsKey("off_delay")) {
    target["off_delay"] = 0.0f;
  }

  if (threshold >= 0) {
    target["tool_on_threshold"] = threshold;
  } else if (!target.containsKey("tool_on_threshold")) {
    target["tool_on_threshold"] = 0.0f;
  }

  file = SPIFFS.open("/registry.json", "w");
  if (file) {
    serializeJson(doc, file);
    String debug;
    serializeJson(doc, debug);
    Serial.printf("Registry updated: %s\n", debug.c_str());
    file.close();
  }

  setStationOffDelay(mac, target["off_delay"] | 0.0f);
  setStationThreshold(mac, target["tool_on_threshold"] | 0.0f);
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
    float newOffDelay = incoming["off_delay"] | -1.0f;
    float newThreshold = incoming["tool_on_threshold"] | -1.0f;
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
    JsonArray arr = doc.is<JsonArray>() ? doc.as<JsonArray>() : doc.to<JsonArray>();

    bool found = false;
    for (JsonObject obj : arr) {
      if (newMac == obj["mac"].as<String>()) {
        obj["name"] = newName;
        if (!newFw.isEmpty()) obj["fw"] = newFw;
        if (newOffDelay >= 0) obj["off_delay"] = newOffDelay;
        if (newThreshold >= 0) obj["tool_on_threshold"] = newThreshold;
        found = true;
        break;
      }
    }

    if (!found) {
      JsonObject entry = arr.add<JsonObject>();
      entry["name"] = newName;     // <- fixed typo: was newObj
      entry["mac"]  = newMac;
      if (!newFw.isEmpty()) entry["fw"] = newFw;
      if (newOffDelay >= 0) entry["off_delay"] = newOffDelay;
      if (newThreshold >= 0) entry["tool_on_threshold"] = newThreshold;
    }

    file = SPIFFS.open("/registry.json", "w");
    serializeJson(doc, file);
    file.close();

    if (newOffDelay >= 0) {
      setStationOffDelay(newMac, newOffDelay);
    }
    if (newThreshold >= 0) {
      setStationThreshold(newMac, newThreshold);
    }

    JsonDocument resp;
    resp["success"] = true;
    String out; serializeJson(resp, out);
    request->send(200, "application/json", out);
  }
  );

  server.on("/registry", HTTP_PATCH, [](AsyncWebServerRequest *request) {}, nullptr,
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

    String mac = incoming["mac"] | "";
    float offDelay = incoming["off_delay"] | -1.0f;
    float threshold = incoming["tool_on_threshold"] | -1.0f;
    if (mac.isEmpty() || (offDelay < 0 && threshold < 0)) {
      JsonDocument doc;
      doc["error"] = "Missing mac or values";
      String out; serializeJson(doc, out);
      request->send(400, "application/json", out);
      return;
    }

    JsonDocument doc;
    File file = SPIFFS.open("/registry.json", "r");
    if (file) {
      deserializeJson(doc, file);
      file.close();
    }

    JsonArray arr = doc.is<JsonArray>() ? doc.as<JsonArray>() : doc.to<JsonArray>();
    bool found = false;
    for (JsonObject obj : arr) {
      if (mac == obj["mac"].as<String>()) {
        if (offDelay >= 0) obj["off_delay"] = offDelay;
        if (threshold >= 0) obj["tool_on_threshold"] = threshold;
        found = true;
        break;
      }
    }

    if (!found) {
      JsonDocument resp;
      resp["error"] = "Not found";
      String out; serializeJson(resp, out);
      request->send(404, "application/json", out);
      return;
    }

    file = SPIFFS.open("/registry.json", "w");
    serializeJson(doc, file);
    file.close();

    if (offDelay >= 0) setStationOffDelay(mac, offDelay);
    if (threshold >= 0) setStationThreshold(mac, threshold);

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

    JsonArray arr = doc.is<JsonArray>() ? doc.as<JsonArray>() : doc.to<JsonArray>();
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

  server.on("/threshold", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (!request->hasParam("mac")) {
      JsonDocument resp; resp["error"] = "Missing mac"; String out; serializeJson(resp, out);
      request->send(400, "application/json", out); return; }
    String mac = request->getParam("mac")->value();
    JsonDocument doc; File file = SPIFFS.open("/registry.json", "r");
    if (file) { deserializeJson(doc, file); file.close(); }
    JsonArray arr = doc.is<JsonArray>() ? doc.as<JsonArray>() : doc.to<JsonArray>();
    for (JsonObject obj : arr) {
      if (mac == obj["mac"].as<String>()) {
        JsonDocument resp; resp["mac"] = mac; resp["tool_on_threshold"] = obj["tool_on_threshold"] | 0.0f; String out; serializeJson(resp, out);
        request->send(200, "application/json", out); return; }
    }
    JsonDocument resp; resp["error"] = "Not found"; String out; serializeJson(resp, out); request->send(404, "application/json", out);
  });

  server.on("/threshold", HTTP_POST, [](AsyncWebServerRequest *request) {}, nullptr,
  [](AsyncWebServerRequest* request, uint8_t* data, size_t len, size_t, size_t){
    JsonDocument incoming; DeserializationError err = deserializeJson(incoming, data, len);
    if (err) { JsonDocument resp; resp["error"] = "Invalid JSON"; String out; serializeJson(resp, out); request->send(400, "application/json", out); return; }
    String mac = incoming["mac"] | ""; float threshold = incoming["tool_on_threshold"] | -1.0f;
    if (mac.isEmpty() || threshold < 0) { JsonDocument resp; resp["error"] = "Missing mac or threshold"; String out; serializeJson(resp, out); request->send(400,"application/json",out); return; }
    updateStationRegistry(mac, "", "", "", -1.0f, threshold);
    JsonDocument resp; resp["success"] = true; String out; serializeJson(resp, out); request->send(200, "application/json", out);
  });

  server.on("/threshold", HTTP_PATCH, [](AsyncWebServerRequest *request) {}, nullptr,
  [](AsyncWebServerRequest* request, uint8_t* data, size_t len, size_t, size_t){
    JsonDocument incoming; DeserializationError err = deserializeJson(incoming, data, len);
    if (err) { JsonDocument resp; resp["error"] = "Invalid JSON"; String out; serializeJson(resp, out); request->send(400, "application/json", out); return; }
    String mac = incoming["mac"] | ""; float threshold = incoming["tool_on_threshold"] | -1.0f;
    if (mac.isEmpty() || threshold < 0) { JsonDocument resp; resp["error"] = "Missing mac or threshold"; String out; serializeJson(resp, out); request->send(400,"application/json",out); return; }
    JsonDocument doc; File file = SPIFFS.open("/registry.json", "r"); if (file) { deserializeJson(doc,file); file.close(); }
    JsonArray arr = doc.is<JsonArray>() ? doc.as<JsonArray>() : doc.to<JsonArray>(); bool found=false; for (JsonObject obj:arr){ if (mac==obj["mac"].as<String>()){ obj["tool_on_threshold"]=threshold; found=true; break; }}
    if (!found) { JsonDocument resp; resp["error"]="Not found"; String out; serializeJson(resp,out); request->send(404,"application/json",out); return; }
    file = SPIFFS.open("/registry.json", "w"); serializeJson(doc,file); file.close(); setStationThreshold(mac, threshold);
    JsonDocument resp; resp["success"] = true; String out; serializeJson(resp,out); request->send(200,"application/json",out);
  });

  server.on("/calibrate", HTTP_POST, [](AsyncWebServerRequest *request) {}, nullptr,
  [](AsyncWebServerRequest* request, uint8_t* data, size_t len, size_t, size_t){
    JsonDocument incoming; DeserializationError err = deserializeJson(incoming, data, len);
    if (err) { JsonDocument resp; resp["error"] = "Invalid JSON"; String out; serializeJson(resp, out); request->send(400, "application/json", out); return; }
    String mac = incoming["mac"] | ""; String step = incoming["step"] | "start";
    if (mac.isEmpty()) { JsonDocument resp; resp["error"] = "Missing mac"; String out; serializeJson(resp,out); request->send(400,"application/json",out); return; }
    JsonDocument resp;
    if (step == "start") {
      sendRecalibrateCommand(mac);
      calibrationSessions[mac] = getStationCurrent(mac);
      resp["next"] = "turn_on";
    } else if (step == "finish") {
      auto it = calibrationSessions.find(mac);
      if (it == calibrationSessions.end()) {
        resp["error"] = "No session"; String out; serializeJson(resp,out); request->send(400,"application/json",out); return; }
      float on = getStationCurrent(mac);
      float threshold = on * 0.3f;
      updateStationRegistry(mac, "", "", "", -1.0f, threshold);
      calibrationSessions.erase(it);
      resp["threshold"] = threshold;
    } else {
      resp["error"] = "Invalid step"; String out; serializeJson(resp,out); request->send(400,"application/json",out); return; }
    String out; serializeJson(resp,out); request->send(200,"application/json",out);
  });
}
