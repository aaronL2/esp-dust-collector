#ifndef SERVO_CONTROL_H
#define SERVO_CONTROL_H

#include <Arduino.h>
#include <ESP32Servo.h>

class ServoControlClass {
public:
    void begin();
    void moveTo(int angle);
    void setGateOpen(bool open);
    bool isGateOpen() const;

private:
    Servo servo;
    bool gateOpen = false;
};

extern ServoControlClass ServoControl;  // this is the instance

#endif
