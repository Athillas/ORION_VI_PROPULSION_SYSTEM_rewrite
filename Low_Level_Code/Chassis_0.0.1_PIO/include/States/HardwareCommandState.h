// HardwareCommandState.h
#ifndef HARDWARE_COMMAND_STATE_H
#define HARDWARE_COMMAND_STATE_H

#include <cstdint>
#include <Arduino.h>

struct HardwareCommandState
{
    volatile float targetVelocity;
    volatile float targetSteering;
    float measuredPos;
    float measuredVel;
    uint32_t activeErrors;

    HardwareCommandState() :
    targetVelocity(0), targetSteering(0),
    measuredPos(0), measuredVel(0), activeErrors(0)
    {}
};

#endif