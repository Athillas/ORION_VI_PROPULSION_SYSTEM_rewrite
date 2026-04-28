// ODriveCAN.h
#ifndef ODRIVE_CAN_H
#define ODRIVE_CAN_H

#include <ArduinoJson.h>
#include <cstdint>

#include "States/HardwareCommandState.h"

namespace ODriveCAN
{
    void initCAN();
    void sendVelocity(uint8_t node_id, float velocity);
    void setAxisState(uint8_t node_id, uint8_t state);
    void setControlMode(uint8_t node_id, uint8_t controlMode, uint8_t inputMode);
    void requestEncoderData(uint8_t node_id);
    void requestODriveErrors(uint8_t node_id);
    void clearErrors(uint8_t node_id);
    void rebootODrive(uint8_t node_id);
    void handleCANMessages(struct HardwareCommandState &hcs); 
}

#endif