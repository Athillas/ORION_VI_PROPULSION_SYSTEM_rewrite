// CANConfig.h
#ifndef CAN_CONFIG_H
#define CAN_CONFIG_H

#include <cstdint>

namespace CANConfig
{
    constexpr uint32_t CAN_BAUD_RATE                        = 250000;
    constexpr uint8_t  ODRIVE_NODE_ID                       = 0;
    
    // ODrive commands
    constexpr uint8_t CMD_ID_GET_ERROR                      = 0x003;
    constexpr uint8_t CMD_ID_SET_AXIS_STATE                 = 0x007;
    constexpr uint8_t CMD_ID_GET_ENCODER                    = 0x009;
    constexpr uint8_t CMD_ID_SET_CONTROLLER_MODE            = 0x00B;
    constexpr uint8_t CMD_ID_SET_INPUT_VEL                  = 0x00D;
    constexpr uint8_t CMD_ID_REBOOT_ODRIVE                  = 0x016;
    constexpr uint8_t CMD_ID_CLEAR_ERRORS                   = 0x018;

    constexpr uint8_t AXIS_STATE_ENCODER_OFFSET_CALIBRATION = 7;
    constexpr uint8_t AXIS_STATE_CLOSED_LOOP_CONTROL        = 8;
    constexpr uint8_t CONTROL_MODE_VELOCITY_CONTROL         = 2;
    constexpr uint8_t INPUT_MODE_PASSTHROUGH                = 1;
    constexpr uint8_t INPUT_MODE_VEL_RAMP                   = 2;

    constexpr uint8_t CAN_CYCLE_DELAY                       = 50;
}

#endif