#include <WiFi.h>
#include <DNSServer.h>
#include <ESPAsyncWebServer.h>
#include "config_portal.h"
#include "net_prefs.h"
//#include "display.h"       // your existing display wrapper
#include "config_ui.h"     // for getFriendlyName() if you have it
#ifdef __has_include
  #if __has_include("generated/version.h")
    #include "generated/version.h"
  #endif
#endif
#ifndef FW_VERSION
  #define FW_VERSION "0.0.0"
#endif
#include "display_status.h"             // <-- this is the helper you already use elsewhere

//extern Display display;    // already defined in your project

static DNSServer dnsServer;
static AsyncWebServer apServer(80);

static void showApPortalOnOled(const char* ap_ssid) {
  const String title = String("AP: ") + ap_ssid;
  const String name  = getFriendlyName();
  const String ip    = WiFi.softAPIP().toString();
  const String mac   = WiFi.softAPmacAddress();
  const String fw    = String(FW_VERSION);
  status_show(title, name, ip, mac, fw);  // ← no class types involved
}

[[maybe_unused]] static bool waitForIP(uint32_t timeoutMs) {
  uint32_t start = millis();
  while ((millis() - start) < timeoutMs) {   // <- use timeoutMs
    if (WiFi.status() == WL_CONNECTED && WiFi.localIP() != IPAddress(0,0,0,0)) {
      return true;
    }
    delay(50);
  }
  return false;
}

void startConfigPortal_Blocking() {
  // Start AP + DNS
  WiFi.mode(WIFI_AP);
  #ifdef DEVICE_ROLE_BASE
    const char* ap_ssid = "DustCollector_Base";
  #else
    const char* ap_ssid = "DustCollector_Station";
  #endif

  WiFi.softAP(ap_ssid);
  IPAddress ip = WiFi.softAPIP();
  dnsServer.start(53, "*", ip);

  // 🔹 New: show AP portal screen on the OLED
  showApPortalOnOled(ap_ssid);

  // Minimal portal page & POST to save creds (replace with your UI)
  apServer.on("/", HTTP_GET, [](AsyncWebServerRequest* req){
    req->send(200, "text/html", "<h1>Config</h1><form method='POST' action='/wifi'>"
                                "SSID: <input name='s'><br>Pass: <input name='p'><br>"
                                "<button>Save</button></form>");
  });
  apServer.on("/wifi", HTTP_POST, [](AsyncWebServerRequest* req){
    auto ps = req->getParam("s", true);
    auto pp = req->getParam("p", true);
    String s = ps ? ps->value() : "";
    String p = pp ? pp->value() : "";
    if (!s.length()) { req->send(400, "text/plain", "SSID required"); return; }

    NetPrefs::save(s, p);
    req->send(200, "text/plain", "Saved. Reconnecting...");
    status_show("Joining Wi-Fi…", getFriendlyName(), "-", "-", String(FW_VERSION));

    dnsServer.stop();
    apServer.end();
    WiFi.softAPdisconnect(true);
    WiFi.mode(WIFI_STA);
    WiFi.begin(s.c_str(), p.c_str());
  });
  apServer.begin();

  // Block until STA has a real IP (or user resets)
  while (true) {
    dnsServer.processNextRequest();
    if (WiFi.getMode() == WIFI_STA && WiFi.status() == WL_CONNECTED &&
        WiFi.localIP() != IPAddress(0,0,0,0)) {
      break; // leave portal
    }
    delay(25);
  }
}
