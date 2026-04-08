// ODriveCAN.h
#ifndef ODRIVE_CAN_H
#define ODRIVE_CAN_H

#include <Arduino.h>
#include <CAN.h>

#include "Config.h"
#include "States/HardwareCommandState.h"

namespace ODriveCAN
{
    void initCAN();
    void sendVelocity(float velocity);
    void setAxisState(int32_t state);
    void setControlMode(int32_t controlMode, int32_t inputMode);
    void requestODriveErrors();
    void requestEncoderData();
    void clearErrors();
    void rebootODrive();
    void handleCANMessages(struct HardwareCommandState &hcs); 
}

#endif