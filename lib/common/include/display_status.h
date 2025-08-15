#pragma once
#include <Arduino.h>
#include <U8g2lib.h>

// Compact 5-line OLED status view (128x64):
// Name, mDNS, IP, MAC, FW
class DisplayStatus {
public:
  explicit DisplayStatus(U8G2& u8) : u8g2(u8) {}

  void begin() {
    u8g2.setFont(u8g2_font_5x8_mf);
    u8g2.setFontRefHeightExtendedText();
    u8g2.setFontPosTop();
  }

  void show(const String& name,
            const String& mdns,
            const String& ip,
            const String& mac,
            const String& fw) {
    if (name == lastName && mdns == lastMdns && ip == lastIp && mac == lastMac && fw == lastFw) return;
    lastName = name; lastMdns = mdns; lastIp = ip; lastMac = mac; lastFw = fw;

    const uint8_t W = 128, padX = 2, padY = 2, lh = 10; // ~8px font + spacing
    const uint8_t maxW = W - 2*padX;

    u8g2.firstPage();
    do {
      uint8_t y = padY;
      u8g2.setCursor(padX, y); u8g2.print(fit(name, maxW));           y += lh;
      u8g2.setCursor(padX, y); u8g2.print(fit("mDNS: " + mdns, maxW)); y += lh;
      u8g2.setCursor(padX, y); u8g2.print(fit("IP: "   + ip,   maxW)); y += lh;
      u8g2.setCursor(padX, y); u8g2.print(fit("MAC: "  + mac,  maxW)); y += lh;
      u8g2.setCursor(padX, y); u8g2.print(fit("FW: "   + fw,   maxW));
    } while (u8g2.nextPage());
  }

private:
  U8G2& u8g2;
  String lastName, lastMdns, lastIp, lastMac, lastFw;

  String fit(const String& s, uint8_t maxPx) {
    if (u8g2.getStrWidth(s.c_str()) <= maxPx) return s;
    String out = s;
    while (out.length() > 1) {
      out.remove(out.length() - 1);
      String test = out + "…";
      if (u8g2.getStrWidth(test.c_str()) <= maxPx) return test;
    }
    return "…";
  }
};

// Simple C-style wrapper so other modules don’t need to know the class type
void status_init(U8G2& u8);
void status_begin();
void status_show(const String& l1,
                 const String& l2,
                 const String& l3,
                 const String& l4,
                 const String& l5);
