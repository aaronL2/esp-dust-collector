#include "display_status.h"

// Singleton-style stored pointer so callers don't need the class/type
static DisplayStatus* gStatus = nullptr;

void status_init(U8G2& u8) {
  static DisplayStatus inst(u8);  // constructed once
  gStatus = &inst;
}

void status_begin() {
  if (gStatus) gStatus->begin();
}

void status_show(const String& l1,
                 const String& l2,
                 const String& l3,
                 const String& l4,
                 const String& l5,
                 const String& l6) {
  if (gStatus) gStatus->show(l1, l2, l3, l4, l5, l6);
}
