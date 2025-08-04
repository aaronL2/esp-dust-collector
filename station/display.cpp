#include "display.h"
#include <U8g2lib.h>
#include "qrcode.h"

U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0);
Display display;

void Display::begin() {
  u8g2.begin();
}

void Display::showDeviceInfo(const String& name) {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB08_tr);
  u8g2.drawStr(0, 10, name.c_str());
  drawQRCode("http://" + name + ".local");
  u8g2.sendBuffer();
}

void Display::drawQRCode(const String& url) {
  QRCode qrcode;
  uint8_t qrcodeData[qrcode_getBufferSize(3)];
  qrcode_initText(&qrcode, qrcodeData, 3, 0, url.c_str());

  const int offset_x = 70;
  const int offset_y = 0;
  const int scale = 2;

  for (uint8_t y = 0; y < qrcode.size; y++) {
    for (uint8_t x = 0; x < qrcode.size; x++) {
      if (qrcode_getModule(&qrcode, x, y)) {
        u8g2.drawBox(offset_x + x * scale, offset_y + y * scale, scale, scale);
      }
    }
  }
}
