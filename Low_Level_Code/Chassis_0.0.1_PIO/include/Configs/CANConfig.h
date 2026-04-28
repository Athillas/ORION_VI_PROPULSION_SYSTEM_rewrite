// CANConfig.h
#ifndef CAN_CONFIG_H
#define CAN_CONFIG_H

#include <cstdint>
#include "Configs/HardwareConfig.h"

namespace CANConfig
{
    constexpr uint32_t CAN_BAUD_RATE                        = 500000; // 500000
    constexpr uint8_t CAN_CYCLE_DELAY                       = 50;

    enum ODriveId : uint8_t 
    {
        FRONT                                               = 0,
        REAR                                                = 1
    };

    enum ODriveFeedbackCommand : uint8_t
    {
        HEARTBEAT                                           = 0x001,
        GET_ERROR                                           = 0x003,
        SET_AXIS_STATE                                      = 0x007,
        GET_ENCODER                                         = 0x009,
        SET_CONTROLLER_MODE                                 = 0x00B,
        SET_INPUT_VEL                                       = 0x00D,
        REBOOT_ODRIVE                                       = 0x016,
        CLEAR_ERRORS                                        = 0x018
    };

    enum ODriveControlPacketCommand : uint8_t
    {
        NO_COMMAND                                          = 0,
        CALIBRATE                                           = 1,
        CLOSED_LOOP                                         = 2,
        SET_VEL_MODE                                        = 3,
        SET_RAMP_MODE                                       = 4,
        DUMP_ERRORS                                         = 5,
        REBOOT_ODRIVE_CMD                                   = 6 // had to add _CMD because of name collision with ODriveCommand::REBOOT_ODRIVE
    };
    
    enum ODriveControlCmd : uint8_t
    {
        AXIS_STATE_ENCODER_OFFSET_CALIBRATION               = 7, //7
        AXIS_STATE_CLOSED_LOOP_CONTROL                      = 8,
        CONTROL_MODE_VELOCITY_CONTROL                       = 2,
        INPUT_MODE_PASSTHROUGH                              = 1,
        INPUT_MODE_VEL_RAMP                                 = 2,
    };

    constexpr inline uint16_t getPacketId(CANConfig::ODriveId id, CANConfig::ODriveFeedbackCommand cmd)
    {
        return (id << 5) | cmd;
    }
}

#endif