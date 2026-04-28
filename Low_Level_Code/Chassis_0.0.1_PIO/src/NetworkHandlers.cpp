// NetworkHandlers.cpp
#include <Arduino.h>
#include <ArduinoJson.h>

#include "NetworkHandlers.h"
#include "ODriveCAN.h"
#include "Network.h"

#include "States/NetworkState.h"
#include "States/HardwareCommandState.h"

#include "Configs/CANConfig.h"
#include "Configs/NetworkConfig.h"

using CANConfig::ODriveCommandId;
using CANConfig::ODriveAxisState;
using CANConfig::ODriveInputMode;

void NetworkHandlers::setVelocityHandler(
    JsonDocument &doc,
    struct NetworkState &ns,
    struct HardwareCommandState &hcs
)
{
    if (doc["velocity"].is<float>())
    {
        hcs.wheels[0].targetVelocity = doc["velocity"];
        hcs.wheels[1].targetVelocity = doc["velocity"];
        Serial.println(hcs.wheels[0].targetVelocity);
    }
    
    if (doc["steering"].is<float>())
    {
        float s = doc["steering"];
        if (s < -1.0f) s = -1.0f;
        if (s > 1.0f) s = 1.0f;
            
        hcs.wheels[0].targetSteering = s;
        hcs.wheels[1].targetSteering = s;
        
        Serial.print("[MQTT] New Steering Target: "); Serial.println(hcs.wheels[1].targetSteering);
    }
}


// ODrive CAN 1&2
// Front & rear wheel
// MQTT config info + adresses for 4 wheels
// Base -> {wheels_left, wheels right}

void NetworkHandlers::controlCmdHandler(JsonDocument &doc,
    HardwareCommandState &hcs,
    NetworkState &ns
)
{
    /*
        wheelCommands: [
            0: left front command,
            1: left rear command,
            2: right front command,
            3: right rear command,
            4: override all command 
        ]
        command:
            0 - no command,
            1 - calibrate,
            2 - closed_loop,
            3 - set_vel_mode,
            4 - set_ramp_mode,
            5 - dump_errors,
            6 - reboot_odrive
    */
    ns.lastMqttCmdTime = millis(); // watchdog actualization

    if(!doc.is<JsonArray>() || doc.as<JsonArray>().size() != 5)
    {
        Serial.println("[MQTT] ERROR: Controller command packet is not a 5 element JsonAray!");
    }

    auto cmd_arr = doc.as<JsonArray>();

    
    Serial.print("[MQTT] Command packet: [");
    for(uint8_t i = 0; i < cmd_arr.size(); i++)
    {
        Serial.print(cmd_arr[i].as<uint8_t>());
        if(i != 4)
        Serial.print(", ");
    }
    Serial.print("]\n");

    ODrivePacketCommand o_drive_commands[2];
    uint8_t startIdx = HardwareConfig::SIDE ? 2 : 0;

    if(cmd_arr[4] == 0) // No override
    {
        o_drive_commands[0] = cmd_arr[startIdx];     // Front
        o_drive_commands[1] = cmd_arr[startIdx + 1]; // Rear
    }
    else // Global override command
    {
        o_drive_commands[0] = o_drive_commands[1] = cmd_arr[4];
    }

    for(uint8_t i = 0; i < 2; i++)
    {
        switch (o_drive_commands[i])
        {
            case ODrivePacketCommand::CALIBRATE:
                if (hcs.wheels[i].axisState != ODriveAxisState::UNDEFINED &&
                    hcs.wheels[i].axisState != ODriveAxisState::IDLE
                )
                {
                    Serial.println("[CAN] ERROR: Incorrect axis state at encoder offset calibratio stage! Reboot the ODrive and try again.");
                    Network::sendErrorMessage(i, 0x100);
                }
                ODriveCAN::clearErrors(i);
                delay(100);
                ODriveCAN::setAxisState(i, ODriveAxisState::ENCODER_OFFSET_CALIBRATION);
                delay(100);
                break;
            case ODrivePacketCommand::CLOSED_LOOP:
                if(hcs.wheels[i].axisState != ODriveAxisState::UNDEFINED &&
                    hcs.wheels[i].axisState != ODriveAxisState::IDLE
                )
                {
                    Serial.println("[CAN] ERROR: Incorrect axis state at closed loop stage! Reboot the ODrive and try again.");
                    break;
                }
                ODriveCAN::clearErrors(i);
                delay(100);
                ODriveCAN::setAxisState(i, ODriveAxisState::CLOSED_LOOP_CONTROL);
                delay(100);
                break;
            case ODrivePacketCommand::SET_VEL_MODE:
                if(hcs.wheels[i].axisState != ODriveAxisState::CLOSED_LOOP_CONTROL)
                {
                    Serial.println("[CAN] ERROR: Incorrect axiss state at set_vel_mode stage! Reboot the ODrive and try again.");
                    break;
                }
                ODriveCAN::clearErrors(i);
                delay(100);
                ODriveCAN::setControlMode(i, CANConfig::CONTROL_MODE_VELOCITY_CONTROL, ODriveInputMode::PASSTHROUGH);
                delay(100);
                break;
            case ODrivePacketCommand::DUMP_ERRORS:
                ODriveCAN::requestODriveErrors(i);
                delay(100);
                Serial.println("[CAN] Requesting error dump...");
                break;
            case ODrivePacketCommand::REBOOT_ODRIVE:
                ODriveCAN::rebootODrive(i);
                delay(100);
                Serial.println("[CAN] Rebooting ODrive...");
        }
    }
}

void NetworkHandlers::feedbackEncHandler(
    struct NetworkState &ns, const struct HardwareCommandState &hcs
)
{
    char msg[128];

    /*
        [
            0: side (0- left|1- right)
            1: measured velocity of the front ODrive
            2: measured position of the front ODrive
            3: measured velocity of the rear ODrive
            4: measured position of the rear ODrive
        ]
    */
   
    snprintf(msg, sizeof(msg), "[%d, %.2f, %.2f, %.2f, %.2f]",
        HardwareConfig::SIDE,
        hcs.wheels[0].measuredVel,
        hcs.wheels[0].measuredPos,
        hcs.wheels[1].measuredVel,
        hcs.wheels[1].measuredPos
    );

    //snprintf(msg, sizeof(msg), "{\"v_meas\":[%.2f],\"p_meas\":[%.2f]}", hcs.measuredVel, hcs.measuredPos);
    ns.client.publish(NetworkConfig::TOPIC_FEEDBACK, msg);
}

void NetworkHandlers::errorEncHandler(
    struct NetworkState &ns,
    const uint8_t node_id,
    const uint32_t errorDesc
)
{
    char errBuf[64];

    // odrive_id: (node_id << 1) | side
    snprintf(errBuf, sizeof(errBuf), "{\"error\": \"0x%X\", \"odrive_id\": \"%d%d\"}",
        errorDesc,
        node_id,
        HardwareConfig::SIDE
    );

    ns.client.publish(NetworkConfig::TOPIC_FEEDBACK, errBuf);
}