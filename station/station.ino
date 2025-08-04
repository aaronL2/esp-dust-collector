#include <WiFi.h>
#include "config_ui.h"
#include "display.h"
#include "servo_control.h"

void setup() {
  Serial.begin(115200);
  WiFi.begin("Landry");
  configUI.begin();
  display.begin();
  display.showDeviceInfo(configUI.getFriendlyName());
  ServoControl.begin();
}

void loop() {
  // Nothing for now
}
