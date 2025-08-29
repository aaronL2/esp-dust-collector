#pragma once
#include <Arduino.h>

// GPIO pin connected to the dust collector relay driver
extern const uint8_t RELAY_PIN;

// Logic level that turns the relay on (set to LOW if your hardware is active-low)
constexpr uint8_t RELAY_ON_LEVEL = HIGH;
