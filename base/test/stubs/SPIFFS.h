#pragma once
#include "FS.h"
class SPIFFSClass {
public:
  File open(const char*, const char*) { return File(); }
};
inline SPIFFSClass SPIFFS;
