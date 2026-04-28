// ODriveCAN.cpp
#include <Arduino.h>
#include <CAN.h>

#include "Pins.h"
#include "ODriveCAN.h"
#include "Network.h"

#include "Configs/CANConfig.h"

#include "States/HardwareCommandState.h"

using CANConfig::ODriveCommandId;

void ODriveCAN::initCAN()
{
    Serial.println("[CAN] Initializing...");
    CAN.setPins(Pins::CAN_RX_PIN, Pins::CAN_TX_PIN);

    if(!CAN.begin(CANConfig::BAUD_RATE))
    {
        Serial.println("[CAN] Initialization failed! Check connections and restart the board.");
        while(1);
    }else
    {
        Serial.println("[CAN] Initialization successsful!");
    }
}

void ODriveCAN::sendVelocity(uint8_t node_id, float velocity)
{
    if(node_id != 0 && node_id != 1) 
    {
        Serial.println("[CAN] Wrong node_id in send velocity!");
        return;
    }

    float torqueFF = 0.0f;

    uint16_t packetId = (node_id << 5) | ODriveCommandId::SET_INPUT_VEL;

    CAN.beginPacket(packetId);
    CAN.write((uint8_t*)&velocity, 4);
    CAN.write((uint8_t*)&torqueFF, 4);

    Serial.println("[CAN] Attempting to send data frame (using endPacket)...");

    if(!CAN.endPacket())
    {
        Serial.println("[CAN] ERROR: Failed to send velocity data frame!");
    }
}

void ODriveCAN::setAxisState(uint8_t node_id, uint8_t state)
{
    uint16_t packet_id = (node_id << 5) | ODriveCommandId::SET_AXIS_STATE;
    CAN.beginPacket(packet_id);
    CAN.write((uint8_t*)&state, 4);
    if(!CAN.endPacket())
    {
        Serial.println("[CAN] ERROR: Failed to send axis state configuration!");
    }
}

void ODriveCAN::setControlMode(uint8_t node_id, uint8_t controlMode, uint8_t inputMode)
{
    uint16_t packet_id = (node_id << 5) | ODriveCommandId::SET_CONTROLLER_MODE;
    
    CAN.beginPacket(packet_id);
    CAN.write((uint8_t*)&controlMode, 4);
    CAN.write((uint8_t*)&inputMode, 4);

    if(!CAN.endPacket())
    {
        Serial.println("[CAN] ERROR: Failed to send controller mode configuration!");
    }
}

void ODriveCAN::requestEncoderData(uint8_t node_id)
{
    uint16_t packetId = (node_id << 5) | ODriveCommandId::GET_ENCODER;
    CAN.beginPacket(packetId, 8, true);
    if(!CAN.endPacket())
    {
        Serial.println("[CAN] ERROR: Failed to send controller mode configuration!");
    }
}

void ODriveCAN::clearErrors(uint8_t node_id)
{
    uint16_t packetId = (node_id << 5) | ODriveCommandId::CLEAR_ERRORS;
    CAN.beginPacket(packetId);
    if(!CAN.endPacket())
    {
        Serial.println("[CAN] ERROR: Failed to send clear errors request!");
    }
}

void ODriveCAN::rebootODrive(uint8_t node_id)
{
    uint16_t packetId = (node_id << 5) | ODriveCommandId::REBOOT_ODRIVE;
    CAN.beginPacket(packetId);
    if(!CAN.endPacket())
    {
        Serial.println("[CAN] ERROR: Failed to send ODrive reboot request!");
    }
}

void ODriveCAN::requestODriveErrors(uint8_t node_id)
{
    uint16_t packet_id = (node_id << 5) | ODriveCommandId::GET_ERROR;

    CAN.beginPacket(packet_id, 0, true);

   if(!CAN.endPacket())
    {
        Serial.println("[CAN] ERROR: Failed to send dump errors request to ODrive!");
    }
}

void ODriveCAN::handleCANMessages(struct HardwareCommandState &hcs)
{
    uint32_t packet_size = CAN.parsePacket();
    if(!packet_size || CAN.packetRtr()) return;

    uint32_t packet_id = CAN.packetId();

    uint8_t cmd_id = packet_id & 0x01F;
    uint8_t node_id = ((packet_id >> 5) & 0x3F); // Extracting bits 5-10
    uint8_t buffer[8]{};

    if(node_id > 2)
    {
        Serial.println("[CAN] CRITICAL ERROR: Node id out of bounds!");
        return;
    }

    for(uint8_t i = 0; i < packet_size && i < 8; i++)
    {
        buffer[i] = CAN.read();
    }

    auto &current_wheel = hcs.wheels[node_id];

    uint32_t current_error;

    switch (cmd_id)
    {
        case ODriveCommandId::HEARTBEAT:
            if(packet_size < 4)
            {
                Serial.println("[CAN] ERROR: Wrong heartbeat packet size");
                break;
            }

            memcpy(&current_error, &buffer[0], 4);

            if(current_error != 0)
            {
                Serial.print("[CAN] Heartbeat -> Error: 0x");
                Serial.print(current_error, HEX);
                Serial.println();
            }

            if(packet_size >= 5)
            {
                uint8_t axis_state = buffer[4];
                Serial.print("[CAN] Axis state: ");
                Serial.print(axis_state);
                current_wheel.axisState = axis_state;
            }

            Serial.println();

            if(current_error != 0 && current_error != current_wheel.activeErrors)
            {
                current_wheel.activeErrors = current_error;
                Serial.print("[CAN] ERROR: ODrive error: 0x");
                Serial.println(current_wheel.activeErrors, HEX);
                Network::sendErrorMessage(node_id, current_wheel.activeErrors);
            }
            else if(current_error == 0 && current_wheel.activeErrors != 0)
            {
                current_wheel.activeErrors = 0;
                Serial.println("[CAN] ODrive errors were cleared.");
            }

            break;
        case ODriveCommandId::GET_ENCODER:
            if(packet_size < 8)
            {
                Serial.println("[CAN] ERROR: Wrong GetEncoder packet size!");
                break;
            }

            memcpy(&current_wheel.measuredPos, &buffer[0], 4);
            memcpy(&current_wheel.measuredVel, &buffer[4], 4);

            break;
        case ODriveCommandId::GET_ERROR:
            if(packet_size < 4)
            {
                Serial.println("[CAN] ERROR: Wrong get error packet size!");
                break;
            }

            uint32_t requested_error = 0;
            memcpy(&requested_error, &buffer[0], 4);

            Serial.print("[CAN] ODrive Error (by request): 0x");
            Serial.println(requested_error, HEX);

            Network::sendErrorMessage(node_id, requested_error);

            break;
    }
}