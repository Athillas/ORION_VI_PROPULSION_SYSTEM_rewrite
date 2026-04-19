// HardwareConfig.h
#ifndef HARDWARE_CONFIG_H
#define HARDWARE_CONFIG_H

#include <cstdint>

namespace HardwareConfig
{
    constexpr uint32_t  SERIAL_BAUD_RATE    = 115200;

    constexpr uint8_t   LEDC_CHANNEL        = 0;
    constexpr uint32_t  LEDC_FREQ           = 50;
    constexpr uint8_t   LEDC_RESOLUTION     = 16;

    constexpr uint16_t  MIN_PULSE           = 500;
    constexpr uint16_t  MAX_PULSE           = 2500;
    constexpr uint16_t  MID_PULSE           = 1500;
    
    constexpr float     SMOOTH_FACTOR       = 0.08;
    constexpr float     DEADBAND            = 3.0;

    constexpr float     STARTING_SERVO_POS  = 1500.0;

    
    enum WheelsSide : uint8_t
    {
        LEFT                                = 0,
        RIGHT                               = 1
    };
    
    constexpr WheelsSide SIDE               = LEFT;
}

#endif