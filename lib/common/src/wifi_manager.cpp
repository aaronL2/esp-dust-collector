#include <WiFi.h>
#include "net_prefs.h"
#include "config_portal.h"

static bool connectWithTimeout(uint32_t ms) {
  uint32_t start = millis();
  while (WiFi.status() != WL_CONNECTED && (millis() - start) < ms) {
    delay(50);
  }
  return WiFi.status() == WL_CONNECTED;
}

// NEW: wait until DHCP assigns a non-0 IP (or timeout)
static bool waitForIP(uint32_t ms) {
  uint32_t start = millis();
  while ((millis() - start) < ms) {
    if (WiFi.status() == WL_CONNECTED && WiFi.localIP() != IPAddress(0,0,0,0)) {
      return true;
    }
    delay(50);
  }
  return false;
}

void setupWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.setAutoConnect(true);
  WiFi.setAutoReconnect(true);
  WiFi.persistent(true);

  String ssid, pass;

  // Try saved creds from NVS first
  if (NetPrefs::load(ssid, pass)) {
    WiFi.begin(ssid.c_str(), pass.c_str());
    if (connectWithTimeout(12000)) {
      waitForIP(5000);  // <-- ensure we have a real IP
      return;
    }
  }

  // Optional fallback to known SSID (comment out if not desired)
  // WiFi.begin("Landry");
  // if (connectWithTimeout(8000)) {
  //   waitForIP(5000);
  //   return;
  // }

  // If we got here, open your captive portal (blocks until creds saved)
  startConfigPortal_Blocking();

  // When portal returns, STA should be attempting to connect:
  // Make sure we really have an IP before continuing.
  connectWithTimeout(15000);
  waitForIP(5000);
}
