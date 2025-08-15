// net_prefs.h
#pragma once
#include <Arduino.h>

namespace NetPrefs {
  bool load(String& ssid, String& pass);
  void save(const String& ssid, const String& pass);
  void clear();
}
