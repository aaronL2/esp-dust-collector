#include "display.h"
#include <U8g2lib.h>
#include "qrcode.h"
#include <WiFi.h>

U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0);
Display display;

void Display::begin() {
  u8g2.begin();
}

void Display::showDeviceInfo(const String& name) {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB08_tr);
  u8g2.drawStr(0, 10, name.c_str());

  IPAddress ip = WiFi.localIP();
  char ipStr[16];
  snprintf(ipStr, sizeof(ipStr), "%s", ip.toString().c_str());

  uint8_t mac[6];
  esp_read_mac(mac, ESP_MAC_WIFI_STA);
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
           mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

  u8g2.setFont(u8g2_font_6x12_tf);
  u8g2.drawStr(0, 22, ipStr);
  u8g2.drawStr(0, 34, macStr);

  u8g2.sendBuffer();  // Ends cleanly without drawing the QR code
}

void Display::showQRCode(const String& url) {
  u8g2.clearBuffer();

  QRCode qrcode;
  uint8_t qrcodeData[qrcode_getBufferSize(3)];
  qrcode_initText(&qrcode, qrcodeData, 3, 0, url.c_str());

  const int scale = 3;  // slightly larger for better readability
  const int offset_x = (128 - qrcode.size * scale) / 2;
  const int offset_y = (64 - qrcode.size * scale) / 2;

  for (uint8_t y = 0; y < qrcode.size; y++) {
    for (uint8_t x = 0; x < qrcode.size; x++) {
      if (qrcode_getModule(&qrcode, x, y)) {
        u8g2.drawBox(offset_x + x * scale, offset_y + y * scale, scale, scale);
      }
    }
  }

  u8g2.sendBuffer();  // update the screen
}
