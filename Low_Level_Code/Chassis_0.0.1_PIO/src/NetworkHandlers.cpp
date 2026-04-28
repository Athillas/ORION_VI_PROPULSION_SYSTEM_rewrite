// NetworkHandlers.cpp
#include <Arduino.h>
#include <ArduinoJson.h>

#include "NetworkHandlers.h"
#include "ODriveCAN.h"

#include "States/NetworkState.h"
#include "States/HardwareCommandState.h"

#include "Configs/CANConfig.h"
#include "Configs/NetworkConfig.h"

void NetworkHandlers::setVelocityHandler(
    StaticJsonDocument<NetworkConfig::MQTT_MAX_JSON_PAYLOAD> &doc,
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


static void handleODriveCommands(JsonArray payload, HardwareCommandState &hcs)
{
    CANConfig::ODriveControlPacketCommand o_drive_commands[2];
    uint8_t startIdx = HardwareConfig::SIDE ? 2 : 0; // Right side starts at index 2, Left at 0

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

    if(payload[4] == 0) // No override
    {
        o_drive_commands[0] = payload[startIdx];     // Front
        o_drive_commands[1] = payload[startIdx + 1]; // Rear
    }
    else // Global override command
    {
        o_drive_commands[0] = o_drive_commands[1] = payload[4];
    }

    switch(o_drive_commands[0])
        {
            case CANConfig::CALIBRATE:
                //if(calibration_stage[0] != NetworkConfig::UNINITIALIZED) break;

                ODriveCAN::clearErrors(static_cast<CANConfig::ODriveId>(0));
                delay(50);
                ODriveCAN::setAxisState(
                    CANConfig::AXIS_STATE_ENCODER_OFFSET_CALIBRATION,
                    static_cast<CANConfig::ODriveId>(0)
                );
                //calibration_stage[0] = NetworkConfig::CALIBRATION;
                break;
            case CANConfig::CLOSED_LOOP:
                //if(calibration_stage[0] != NetworkConfig::CALIBRATION) break;

                //ODriveCAN::clearErrors(static_cast<CANConfig::ODriveId>(0));
                delay(50);
                ODriveCAN::setAxisState(
                    CANConfig::AXIS_STATE_CLOSED_LOOP_CONTROL,
                    static_cast<CANConfig::ODriveId>(0)
                );
                //calibration_stage[0] = NetworkConfig::CLOSED_LOOP;
                break;
            case CANConfig::SET_VEL_MODE:
                //if(calibration_stage[0] != NetworkConfig::CLOSED_LOOP) break;

                //ODriveCAN::clearErrors(static_cast<CANConfig::ODriveId>(0));
                delay(50);
                ODriveCAN::setControlMode(
                    CANConfig::CONTROL_MODE_VELOCITY_CONTROL,
                    CANConfig::INPUT_MODE_PASSTHROUGH,
                    static_cast<CANConfig::ODriveId>(0)
                );
                Serial.println(">> Tryb: VELOCITY PASSTHROUGH");

                //calibration_stage[0] = NetworkConfig::VELOCITY_MODE;
                break;
            case CANConfig::SET_RAMP_MODE:
                //if(calibration_stage[0] != NetworkConfig::VELOCITY_MODE) break;
                //ODriveCAN::clearErrors(static_cast<CANConfig::ODriveId>(0));
                delay(50);
                ODriveCAN::setControlMode(
                    CANConfig::CONTROL_MODE_VELOCITY_CONTROL,
                    CANConfig::INPUT_MODE_VEL_RAMP,
                    static_cast<CANConfig::ODriveId>(0)
                );
                Serial.println(">> Tryb: VELOCITY RAMP");
                //calibration_stage[0] = NetworkConfig::RAMP_MODE;
                break;
            case CANConfig::DUMP_ERRORS:
                ODriveCAN::requestODriveErrors(static_cast<CANConfig::ODriveId>(0));
                delay(50);
                Serial.println(">> ODrive: Requesting Error Dump...");
                break;
            case CANConfig::REBOOT_ODRIVE_CMD:
                //calibration_stage[0] = NetworkConfig::UNINITIALIZED;
                ODriveCAN::clearErrors(static_cast<CANConfig::ODriveId>(0));
                delay(50);
                ODriveCAN::rebootODrive(static_cast<CANConfig::ODriveId>(0));
                Serial.println(">> REBOOTING ODRIVE...");
                break;
            default:
                Serial.println("[MQTT] Critical! Unknown command");
                break;
    }

    // for(uint8_t i = 0; i < 2; i++)
    // {
    //     switch(o_drive_commands[i])
    //     {
    //         case CANConfig::CALIBRATE:
    //             if(calibration_stage[i] != NetworkConfig::UNINITIALIZED) break;

    //             ODriveCAN::clearErrors(static_cast<CANConfig::ODriveId>(i));
    //             delay(50);
    //             ODriveCAN::setAxisState(
    //                 CANConfig::AXIS_STATE_ENCODER_OFFSET_CALIBRATION,
    //                 static_cast<CANConfig::ODriveId>(i)
    //             );
    //             calibration_stage[i] = NetworkConfig::CALIBRATION;
    //             break;
    //         case CANConfig::CLOSED_LOOP:
    //             if(calibration_stage[i] != NetworkConfig::CALIBRATION) break;

    //             ODriveCAN::clearErrors(static_cast<CANConfig::ODriveId>(i));
    //             delay(50);
    //             ODriveCAN::setAxisState(
    //                 CANConfig::AXIS_STATE_CLOSED_LOOP_CONTROL,
    //                 static_cast<CANConfig::ODriveId>(i)
    //             );
    //             calibration_stage[i] = NetworkConfig::CLOSED_LOOP;
    //             break;
    //         case CANConfig::SET_VEL_MODE:
    //             if(calibration_stage[i] != NetworkConfig::CLOSED_LOOP) break;

    //             ODriveCAN::clearErrors(static_cast<CANConfig::ODriveId>(i));
    //             delay(50);
    //             ODriveCAN::setControlMode(
    //                 CANConfig::CONTROL_MODE_VELOCITY_CONTROL,
    //                 CANConfig::INPUT_MODE_PASSTHROUGH,
    //                 static_cast<CANConfig::ODriveId>(i)
    //             );
    //             Serial.println(">> Tryb: VELOCITY PASSTHROUGH");

    //             calibration_stage[i] = NetworkConfig::VELOCITY_MODE;
    //             break;
    //         case CANConfig::SET_RAMP_MODE:
    //             if(calibration_stage[i] != NetworkConfig::VELOCITY_MODE) break;
    //             ODriveCAN::clearErrors(static_cast<CANConfig::ODriveId>(i));
    //             delay(50);
    //             ODriveCAN::setControlMode(
    //                 CANConfig::CONTROL_MODE_VELOCITY_CONTROL,
    //                 CANConfig::INPUT_MODE_VEL_RAMP,
    //                 static_cast<CANConfig::ODriveId>(i)
    //             );
    //             Serial.println(">> Tryb: VELOCITY RAMP");
    //             calibration_stage[i] = NetworkConfig::RAMP_MODE;
    //             break;
    //         case CANConfig::DUMP_ERRORS:
    //             ODriveCAN::requestODriveErrors(static_cast<CANConfig::ODriveId>(i));
    //             delay(50);
    //             Serial.println(">> ODrive: Requesting Error Dump...");
    //             break;
    //         case CANConfig::REBOOT_ODRIVE_CMD:
    //             calibration_stage[i] = NetworkConfig::UNINITIALIZED;
    //             ODriveCAN::clearErrors(static_cast<CANConfig::ODriveId>(i));
    //             delay(50);
    //             ODriveCAN::rebootODrive(static_cast<CANConfig::ODriveId>(i));
    //             Serial.println(">> REBOOTING ODRIVE...");
    //             break;
    //         default:
    //             Serial.println("[MQTT] Critical! Unknown command");
    //             break;
    //     }
    // }
}

void NetworkHandlers::controlCmdHandler(StaticJsonDocument<NetworkConfig::MQTT_MAX_JSON_PAYLOAD> &doc, HardwareCommandState &hcs)
{
    if(!doc.is<JsonArray>())
    {
        Serial.println("[MQTT] Wrong JSON format! Not an unsigned integer array of size 5");
        return;
    }

    JsonArray payload = doc.as<JsonArray>();

    if(payload.size() != 5)
    {
        Serial.println("[MQTT] Wrong JSON format! Expected an unsigned integer array of 5 elements, instead length is");
        Serial.println(payload.size());
        return;
    }

    for(uint8_t i = 0; i < payload.size(); i++)
    {
        if(!payload[i].is<unsigned int>())
        {
            Serial.println("[MQTT] Wrong JSON format! Not an integer array");
            return;
        }

        if(payload[i] > 6)
        {
            Serial.print("[MQTT] Unknown command at payload[" );Serial.print(i);
            Serial.print("]. Accepted command range is 0-6");
            return;
        }
    }

    handleODriveCommands(payload, hcs);
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
    const CANConfig::ODriveId node_id,
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