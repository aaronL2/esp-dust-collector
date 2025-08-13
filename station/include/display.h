#pragma once
#include <U8g2lib.h>

class Display {
public:
  void begin();
  U8G2& getU8g2();
};

extern Display display;
