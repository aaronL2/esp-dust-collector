#pragma once
class WiFiClass {
public:
  void mode(int) {}
};
inline WiFiClass WiFi;
constexpr int WIFI_STA = 0;
