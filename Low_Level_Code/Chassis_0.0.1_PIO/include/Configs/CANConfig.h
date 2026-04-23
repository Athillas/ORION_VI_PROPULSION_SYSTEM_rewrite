// CANConfig.h
#ifndef CAN_CONFIG_H
#define CAN_CONFIG_H

#include <cstdint>
#include "Configs/HardwareConfig.h"

namespace CANConfig
{
    constexpr uint32_t CAN_BAUD_RATE                        = 500000;
    constexpr uint8_t CAN_CYCLE_DELAY                       = 50;
    
    enum ODriveId : uint8_t 
    {
        FRONT                                               = 0,
        REAR                                                = 1
    };

    enum ODriveCommand : uint8_t
    {
        GET_ERROR                                           = 0x003,
        SET_AXIS_STATE                                      = 0x007,
        GET_ENCODER                                         = 0x009,
        SET_CONTROLLER_MODE                                 = 0x00B,
        SET_INPUT_VEL                                       = 0x00D,
        REBOOT_ODRIVE                                       = 0x016,
        CLEAR_ERRORS                                        = 0x018,
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
        AXIS_STATE_ENCODER_OFFSET_CALIBRATION               = 7,
        AXIS_STATE_CLOSED_LOOP_CONTROL                      = 8,
        CONTROL_MODE_VELOCITY_CONTROL                       = 2,
        INPUT_MODE_PASSTHROUGH                              = 1,
        INPUT_MODE_VEL_RAMP                                 = 2,
    };

    constexpr inline uint8_t getPacketId(CANConfig::ODriveId id, CANConfig::ODriveCommand cmd)
    {
        return (id << 5) | cmd;
    }

    // enum PacketId : uint8_t
    // {
    //     GET_ERRORS				                            = (CANConfig::ODriveId::FRONT << 5) | CANConfig::ODriveCommand::CMD_ID_GET_ERROR,
    //     SET_AXIS_STATE 			                            = (CANConfig::ODriveId::FRONT << 5) | CANConfig::ODriveCommand::CMD_ID_SET_AXIS_STATE,
    //     GET_ENCODER				                            = (CANConfig::ODriveId::FRONT << 5) | CANConfig::ODriveCommand::CMD_ID_GET_ENCODER,
    //     SET_CONTROL_MODE 		                            = (CANConfig::ODriveId::FRONT << 5) | CANConfig::ODriveCommand::CMD_ID_SET_CONTROLLER_MODE,
    //     SET_INPUT_VEL 			                            = (CANConfig::ODriveId::FRONT << 5) | CANConfig::ODriveCommand::CMD_ID_SET_INPUT_VEL,
    //     REBOOT_ODRIVE			                            = (CANConfig::ODriveId::FRONT << 5) | CANConfig::ODriveCommand::CMD_ID_REBOOT_ODRIVE,
    //     CLEAR_ERRORS			                                = (CANConfig::ODriveId::FRONT << 5) | CANConfig::ODriveCommand::CMD_ID_CLEAR_ERRORS,
    //     REQUEST_ODRIVE_ERRORS 	                            = (CANConfig::ODriveId::FRONT << 5) | CANConfig::ODriveCommand::CMD_ID_GET_ERROR,
    // };
}

#endif