
#include <WiFi.h>
#include <ESPmDNS.h>
#include <Wire.h>
#include <U8g2lib.h>
#include <ESPAsyncWebServer.h>
#include "qrcode.h"

#define RELAY_PIN 5

const char* ssid = "Landry";
const char* password = "1122334455";
const char* hostname = "base";

AsyncWebServer server(80);

U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0);

void showInfo() {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x10_tf);
  u8g2.drawStr(0, 10, "ESP32 BASE");
  u8g2.drawStr(0, 22, ("Name: " + String(hostname)).c_str());
  u8g2.drawStr(0, 34, ("IP: " + WiFi.localIP().toString()).c_str());
  u8g2.drawStr(0, 46, ("MAC: " + WiFi.macAddress()).c_str());
  u8g2.sendBuffer();
}

void showQRCodeU8g2(String url) {
  QRCode qrcode;
  uint8_t qrcodeData[qrcode_getBufferSize(3)];
  qrcode_initText(&qrcode, qrcodeData, 3, ECC_LOW, url.c_str());

  u8g2.clearBuffer();
  int scale = 2;
  int offset_x = (128 - qrcode.size * scale) / 2;
  int offset_y = (64 - qrcode.size * scale) / 2;

  for (uint8_t y = 0; y < qrcode.size; y++) {
    for (uint8_t x = 0; x < qrcode.size; x++) {
      if (qrcode_getModule(&qrcode, x, y)) {
        u8g2.drawBox(offset_x + x * scale, offset_y + y * scale, scale, scale);
      }
    }
  }
  u8g2.sendBuffer();
}

void setup() {
  Serial.begin(115200);
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);

  u8g2.begin();

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected!");
  Serial.println("IP: " + WiFi.localIP().toString());

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
  digitalWrite(RELAY_PIN, HIGH);
  delay(2000);
  digitalWrite(RELAY_PIN, LOW);
  delay(3000);
}
