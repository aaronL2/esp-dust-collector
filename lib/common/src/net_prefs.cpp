// net_prefs.cpp
#include "net_prefs.h"
#include <Preferences.h>

static const char* kNs = "net";

bool NetPrefs::load(String& ssid, String& pass) {
  Preferences p;
  if (!p.begin(kNs, /*readOnly=*/true)) return false;
  if (!p.isKey("ssid")) {
    ssid = "";
    pass = "";
    p.end();
    return false;
  }
  ssid = p.getString("ssid", "");
  pass = p.getString("pass", "");
  p.end();
  return ssid.length() > 0;
}

void NetPrefs::save(const String& ssid, const String& pass) {
  Preferences p;
  if (p.begin(kNs, false)) {
    p.putString("ssid", ssid);
    p.putString("pass", pass);
    p.end();
  }
}

void NetPrefs::clear() {
  Preferences p;
  if (p.begin(kNs, false)) {
    p.clear();
    p.end();
  }
}
