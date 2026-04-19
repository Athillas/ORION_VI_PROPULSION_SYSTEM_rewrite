// ODriveCAN.h
#ifndef ODRIVE_CAN_H
#define ODRIVE_CAN_H

#include <Arduino.h>
#include <CAN.h>

#include "States/HardwareCommandState.h"
#include "Configs/CANConfig.h"

namespace ODriveCAN
{
    void initCAN();
    void sendVelocity(float velocity, CANConfig::ODriveId id);
    void setAxisState(int32_t state, CANConfig::ODriveId id);
    void setControlMode(int32_t controlMode, int32_t inputMode, CANConfig::ODriveId id);
    void requestODriveErrors(CANConfig::ODriveId id);
    void requestEncoderData(CANConfig::ODriveId id);
    void clearErrors(CANConfig::ODriveId id);
    void rebootODrive(CANConfig::ODriveId id);
    void handleCANMessages(struct HardwareCommandState &hcs); 
}

#endif