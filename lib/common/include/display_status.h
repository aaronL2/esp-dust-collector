#pragma once
#include <Arduino.h>
#include <U8g2lib.h>   // U8G2 display instance is provided by the app

// Simple, shared OLED status screen for 128x64 displays.
// Renders:
//  FriendlyName
//  mDNS
//  IP
//  MAC
//  FW: <version>
class DisplayStatus {
public:
  explicit DisplayStatus(U8G2& u8) : u8g2(u8) {}

  // Call once after you call u8g2.begin() in your app
  void begin() {
    u8g2.setFont(u8g2_font_5x8_mf);   // compact, readable, fits 5 lines cleanly
    u8g2.setFontRefHeightExtendedText();
    u8g2.setFontPosTop();
  }

  // Draws only if content changed (reduces flicker / work)
  void show(const String& name,
            const String& mdns,
            const String& ip,
            const String& mac,
            const String& fw) {
    if (name == lastName && mdns == lastMdns && ip == lastIp && mac == lastMac && fw == lastFw) return;
    lastName = name; lastMdns = mdns; lastIp = ip; lastMac = mac; lastFw = fw;

    const uint8_t W = 128, H = 64, padX = 2, padY = 2, lh = 10; // line height ~8px font + 2px spacing
    const uint8_t maxW = W - 2*padX;

    u8g2.firstPage();
    do {
      uint8_t y = padY;

      drawLine(fit(name, maxW));
      y += lh;
      u8g2.setCursor(padX, y); u8g2.print(fit("mDNS: " + mdns, maxW));
      y += lh;
      u8g2.setCursor(padX, y); u8g2.print(fit("IP: " + ip, maxW));
      y += lh;
      u8g2.setCursor(padX, y); u8g2.print(fit("MAC: " + mac, maxW));
      y += lh;
      u8g2.setCursor(padX, y); u8g2.print(fit("FW: " + fw, maxW));
    } while (u8g2.nextPage());
  }

private:
  U8G2& u8g2;

  String lastName, lastMdns, lastIp, lastMac, lastFw;

  void drawLine(const String& s) {
    u8g2.setCursor(2, 2);
    u8g2.print(fit(s, 124));
  }

  // Pixel-accurate end-ellipsis fitting
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
