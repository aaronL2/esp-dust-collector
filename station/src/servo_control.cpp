#include "servo_control.h"

const int SERVO_PIN = 13;
const int OPEN_ANGLE = 0;
const int CLOSE_ANGLE = 90;

ServoControlClass ServoControl;  // define the instance

void ServoControlClass::begin() {
    servo.attach(SERVO_PIN);
    setGateOpen(true);
}

void ServoControlClass::moveTo(int angle) {
    if (angle < 0) angle = 0;
    if (angle > 180) angle = 180;
    servo.write(angle);
}

void ServoControlClass::setGateOpen(bool open) {
    gateOpen = open;
    servo.write(open ? OPEN_ANGLE : CLOSE_ANGLE);
}

bool ServoControlClass::isGateOpen() const {
    return gateOpen;
}
