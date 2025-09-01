#pragma once
#include <string>
#include <type_traits>
#include "Arduino.h"

struct JsonDocument {
  std::string type;
  std::string mac;
  float amps = 0.0f;
  std::string token;

  struct Proxy {
    JsonDocument* doc;
    std::string key;
    Proxy(JsonDocument* d, const std::string& k) : doc(d), key(k) {}

    Proxy& operator=(const char* val) {
      if (key == "type") doc->type = val;
      else if (key == "token") doc->token = val;
      else if (key == "mac") doc->mac = val;
      return *this;
    }

    Proxy& operator=(const String& val) {
      return operator=(val.c_str());
    }

    operator String() const {
      if (key == "type") return doc->type;
      if (key == "mac") return doc->mac;
      if (key == "token") return doc->token;
      return String();
    }

    template <typename T>
    T operator|(T def) const {
      if constexpr (std::is_same_v<T, float>) {
        if (key == "amps") return doc->amps;
      } else {
        if (key == "mac") {
          return doc->mac.empty() ? def : T(doc->mac.c_str());
        }
        if (key == "type") {
          return doc->type.empty() ? def : T(doc->type.c_str());
        }
        if (key == "token") {
          return doc->token.empty() ? def : T(doc->token.c_str());
        }
      }
      return def;
    }

    bool operator==(const char* other) const {
      return String(*this) == other;
    }
  };

  Proxy operator[](const char* k) { return Proxy(this, k); }
};

struct DeserializationError {
  bool failed;
  operator bool() const { return failed; }
};

struct TestMessage {
  const char* type;
  float amps;
  const char* mac;
};

inline DeserializationError deserializeJson(JsonDocument& doc, const uint8_t* data,
                                           size_t) {
  const TestMessage* msg = reinterpret_cast<const TestMessage*>(data);
  doc.type = msg->type ? msg->type : "";
  doc.amps = msg->amps;
  doc.mac = msg->mac ? msg->mac : "";
  return {false};
}

inline size_t serializeJson(const JsonDocument&, uint8_t*) { return 0; }
inline size_t serializeJson(const JsonDocument&, HardwareSerial&) { return 0; }
