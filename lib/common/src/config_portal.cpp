#include <WiFi.h>
#include <DNSServer.h>
#include <ESPAsyncWebServer.h>
#include <ESPmDNS.h>
#include "config_portal.h"
#include "net_prefs.h"
//#include "display.h"       // your existing display wrapper
#include <config_ui.h>
#include <vector>
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

static std::vector<String> scannedSsids;

static void doScan() {
  int n = WiFi.scanNetworks();
  scannedSsids.clear();
  for (int i = 0; i < n; ++i) {
    scannedSsids.push_back(WiFi.SSID(i));
  }
}

static void showApPortalOnOled(const char* ap_ssid) {
  const String title = String("AP: ") + ap_ssid;
  const String name  = configUI.getFriendlyName();
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

  // Scan nearby networks for initial SSID list
  doScan();

  // 🔹 New: show AP portal screen on the OLED
  showApPortalOnOled(ap_ssid);

  // Minimal portal page & POST to save creds (replace with your UI)
  apServer.on("/", HTTP_GET, [](AsyncWebServerRequest* req){
    req->send(200, "text/html", R"HTML(
      <h1>Config</h1>
      <form method='POST' action='/wifi'>
        SSID: <select name='s' id='ssid'></select><br>
        Pass: <input name='p' type='password'><br>
        <button>Save</button>
      </form>
      <button onclick='rescan()'>Rescan</button>
      <script>
        async function load(refresh){
          const url = refresh ? '/scan?refresh=1' : '/scan';
          const res = await fetch(url);
          const list = await res.json();
          const sel = document.getElementById('ssid');
          sel.innerHTML = list.map(s => `<option>${s}</option>`).join('');
        }
        function rescan(){ load(true); }
        load();
      </script>
    )HTML");
  });
  apServer.on("/wifi", HTTP_POST, [](AsyncWebServerRequest* req){
    auto ps = req->getParam("s", true);
    auto pp = req->getParam("p", true);
    String s = ps ? ps->value() : "";
    String p = pp ? pp->value() : "";
    if (!s.length()) { req->send(400, "text/plain", "SSID required"); return; }

    NetPrefs::save(s, p);
    status_show("Joining Wi-Fi…", configUI.getFriendlyName(), "-", "-", String(FW_VERSION));

    WiFi.mode(WIFI_AP_STA);
    WiFi.begin(s.c_str(), p.c_str());

    if (waitForIP(20000)) {
      String target = String("http://") + WiFi.localIP().toString() + "/";
      if (MDNS.isRunning()) {
        target = String("http://") + configUI.getMdnsName() + ".local/";
      }
      String html = String("<!DOCTYPE html><html><head><meta http-equiv='refresh' content='0;url=") + target + "'/></head><body>Redirecting...</body></html>";
      req->send(200, "text/html", html);
    } else {
      req->send(500, "text/plain", "Connection failed");
    }

    dnsServer.stop();
    apServer.end();
    WiFi.softAPdisconnect(true);
    WiFi.mode(WIFI_STA);
  });
  apServer.on("/scan", HTTP_GET, [](AsyncWebServerRequest* req){
    if (req->hasParam("refresh")) {
      doScan();
    }
    String json = "[";
    for (size_t i = 0; i < scannedSsids.size(); ++i) {
      if (i) json += ',';
      json += '"';
      json += scannedSsids[i];
      json += '"';
    }
    json += ']';
    req->send(200, "application/json", json);
  });

  // Handlers for common captive-portal detection endpoints
  auto redirectToRoot = [](AsyncWebServerRequest* req){
    req->redirect("/");
  };
  apServer.on("/generate_204", HTTP_GET, redirectToRoot);
  apServer.on("/hotspot-detect.html", HTTP_GET, redirectToRoot);
  apServer.on("/ncsi.txt", HTTP_GET, redirectToRoot);
  apServer.on("/connecttest.txt", HTTP_GET, redirectToRoot);
  apServer.on("/redirect", HTTP_GET, redirectToRoot);
  apServer.on("/success.txt", HTTP_GET, redirectToRoot);
  apServer.on("/kindle-wifi/wifiredirect.html", HTTP_GET, redirectToRoot);
  apServer.on("/fwlink", HTTP_GET, redirectToRoot);
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
