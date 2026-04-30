// CANConfig.h
#ifndef CAN_CONFIG_H
#define CAN_CONFIG_H

#include <cstdint>
#include "Configs/HardwareConfig.h"

namespace CANConfig
{
    constexpr uint32_t BAUD_RATE                    = 500000;
    constexpr uint32_t CAN_CYCLE_TIMEOUT            = 50;
    
    enum ODriveId : uint8_t
    {
        FRONT                                       = 0x00,
        REAR                                        = 0x01
    };

    enum ODriveCommandId : uint8_t
    {
        HEARTBEAT                                   = 0x001,
        GET_ERROR                                   = 0x003,
        SET_AXIS_STATE                              = 0x007,
        GET_ENCODER                                 = 0x009,
        SET_CONTROLLER_MODE                         = 0x00B,
        SET_INPUT_VEL                               = 0x00D,
        REBOOT_ODRIVE                               = 0x016,
        CLEAR_ERRORS                                = 0x018
    };
    
    constexpr uint8_t CONTROL_MODE_VELOCITY_CONTROL = 2;

    enum ODriveAxisState : uint8_t
    {
        UNDEFINED                                   = 0x0,
        IDLE                                        = 0x1,
        ENCODER_OFFSET_CALIBRATION                  = 0x7,
        CLOSED_LOOP_CONTROL                         = 0x8
    };

    enum ODriveInputMode : uint8_t
    {
        PASSTHROUGH                                 = 1,
        VEL_RAMP                                    = 2
    };
}

#endif