#include "display.h"
#include "display_status.h"
#include <U8g2lib.h>

// Adjust ctor to match your wiring if different:
static U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0);

Display display;

void Display::begin() {
  u8g2.begin();
  // any extra init (flip, contrast, etc.)
  status_init(u8g2);
  status_begin();

}

U8G2& Display::getU8g2() { return u8g2; }
