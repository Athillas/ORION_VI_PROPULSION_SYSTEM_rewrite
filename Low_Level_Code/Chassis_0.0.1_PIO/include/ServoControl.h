// ServoControl.h
#ifndef SERVO_CONTROL_H
#define SERVO_CONTROL_H

#include <Arduino.h>

#include "States/HardwareCommandState.h"

namespace ServoControl
{
    void initServo(struct HardwareCommandState &hcs);
    void setServoMicroseconds(int us);
    void servoTask(void * parameter); // Zadanie FreeRTOS
}

#endif