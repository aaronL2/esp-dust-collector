#include <WiFi.h>
#include <ESPmDNS.h>
#include <Wire.h>
#include <U8g2lib.h>
#include <ESPAsyncWebServer.h>
#include "qrcode.h"

#define RELAY_PIN 5
#define QR_VERSION 3
#define QR_BUF_SIZE 138  // qrcode_getBufferSize(3)

const char* ssid = "Landry";
const char* password = "1122334455";
const char* hostname = "base";

AsyncWebServer server(80);

U8G2_SSD1306_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0);  // Page buffer mode
uint8_t qrcodeData[QR_BUF_SIZE];
QRCode qrcode;

void showInfo() {
  u8g2.firstPage();
  do {
    u8g2.setFont(u8g2_font_6x10_tf);
    u8g2.drawStr(0, 10, "ESP32 BASE");
    u8g2.drawStr(0, 22, ("Name: " + String(hostname)).c_str());
    u8g2.drawStr(0, 34, ("IP: " + WiFi.localIP().toString()).c_str());
    u8g2.drawStr(0, 46, ("MAC: " + WiFi.macAddress()).c_str());
  } while (u8g2.nextPage());
}

void showQRCodeU8g2(String url) {
  qrcode_initText(&qrcode, qrcodeData, QR_VERSION, ECC_LOW, url.c_str());

  int scale = 2;
  int offset_x = (128 - qrcode.size * scale) / 2;
  int offset_y = (64 - qrcode.size * scale) / 2;

  u8g2.firstPage();
  do {
    for (uint8_t y = 0; y < qrcode.size; y++) {
      for (uint8_t x = 0; x < qrcode.size; x++) {
        if (qrcode_getModule(&qrcode, x, y)) {
          u8g2.drawBox(offset_x + x * scale, offset_y + y * scale, scale, scale);
        }
      }
    }
  } while (u8g2.nextPage());
}

void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.println("Booting setup...");
  
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);

  Serial.println("Initializing OLED...");
  u8g2.begin();

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected!");
  Serial.print("IP: "); Serial.println(WiFi.localIP());

  if (MDNS.begin(hostname)) {
    Serial.println("mDNS responder started");
  }

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    String page = "<html><head><title>ESP32 Base</title></head><body>";
    page += "<h1>ESP32 Base</h1>";
    page += "<p><strong>Hostname:</strong> base.local</p>";
    page += "<p><strong>IP:</strong> " + WiFi.localIP().toString() + "</p>";
    page += "<p><strong>MAC:</strong> " + WiFi.macAddress() + "</p>";
    page += "<p><strong>Status:</strong> Relay is toggling every 5s</p>";
    page += "</body></html>";
    request->send(200, "text/html", page);
  });

  server.begin();

  showInfo();
  delay(3000);
  showQRCodeU8g2("http://base.local");
}

void loop() {
  static unsigned long lastToggle = 0;
  static bool relayState = false;
  unsigned long now = millis();

  if (relayState && now - lastToggle >= 2000) {
    relayState = false;
    digitalWrite(RELAY_PIN, LOW);
    lastToggle = now;
  } else if (!relayState && now - lastToggle >= 3000) {
    relayState = true;
    digitalWrite(RELAY_PIN, HIGH);
    lastToggle = now;
  }

  delay(10);  // Yield to avoid watchdog reset
}
