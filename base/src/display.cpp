#include "display.h"
#include <U8g2lib.h>
#include <qrcode.h>

U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0); // adjust if needed

void Display::begin() {
  u8g2.begin();
  lastSwitchTime = millis();
}

void Display::update(const String& n, const String& i, const String& m) {
  name = n;
  ip = i;
  mac = m;
}

void Display::loop() {
  if (millis() - lastSwitchTime > 5000) {
    showingInfo = !showingInfo;
    lastSwitchTime = millis();
    Serial.println(showingInfo ? "Showing info screen" : "Showing QR code");
  }
  if (showingInfo) showDeviceInfo();
  else showQRCode();
}

void Display::showDeviceInfo() {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x12_tr);
  u8g2.drawStr(0, 12, name.c_str());
  u8g2.drawStr(0, 24, ip.c_str());
  u8g2.drawStr(0, 36, mac.c_str());

  String mdns = name + ".local";
  u8g2.drawStr(0, 48, mdns.c_str());

  u8g2.sendBuffer();
}

void Display::showQRCode() {
  String url = "http://" + name + ".local";
  QRCode qrcode;
  uint8_t qrcodeData[qrcode_getBufferSize(3)];
  qrcode_initText(&qrcode, qrcodeData, 3, 0, url.c_str());

  const int scale = 2;
  const int offset_x = (128 - qrcode.size * scale) / 2;
  const int offset_y = (64 - qrcode.size * scale) / 2;

  u8g2.clearBuffer();
  for (uint8_t y = 0; y < qrcode.size; y++) {
    for (uint8_t x = 0; x < qrcode.size; x++) {
      if (qrcode_getModule(&qrcode, x, y)) {
        u8g2.drawBox(offset_x + x * scale, offset_y + y * scale, scale, scale);
      }
    }
  }
  u8g2.sendBuffer();
}
