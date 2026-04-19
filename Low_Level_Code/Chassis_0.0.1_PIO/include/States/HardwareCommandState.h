// HardwareCommandState.h
#ifndef HARDWARE_COMMAND_STATE_H
#define HARDWARE_COMMAND_STATE_H

#include <cstdint>
#include <Arduino.h>

struct HardwareCommandState
{
    struct ODriveWheel
    {
        volatile float targetVelocity;
        volatile float targetSteering;
        float measuredPos;
        float measuredVel;
        uint32_t activeErrors;
    };

    ODriveWheel wheels[2];

    HardwareCommandState()
    {
        wheels[0] = {0.0, 0.0, 0.0, 0.0, 0};
        wheels[1] = {0.0, 0.0, 0.0, 0.0, 0};
    }
};

#endif