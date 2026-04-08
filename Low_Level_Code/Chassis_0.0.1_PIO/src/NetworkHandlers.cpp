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
    StaticJsonDocument<NetworkConfig::MAX_JSON_PAYLOAD> &doc,
    struct NetworkState &ns,
    struct HardwareCommandState &hcs
)
{
    if (doc["velocity"].is<float>())
    {
        hcs.targetVelocity = doc["velocity"];
        Serial.println(hcs.targetVelocity);
    }
    
    if (doc["steering"].is<float>())
    {
        float s = doc["steering"];
        if (s < -1.0f) s = -1.0f;
        if (s > 1.0f) s = 1.0f;
            
        hcs.targetSteering = s;
        Serial.print("[MQTT] New Steering Target: "); Serial.println(hcs.targetSteering);
    }
}

void NetworkHandlers::controlCmdHandler(char *payload)
{
    if (strcmp(payload, "calibrate") == 0)
    {
        ODriveCAN::setAxisState(CANConfig::AXIS_STATE_ENCODER_OFFSET_CALIBRATION);
    }
    else if (strcmp(payload, "closed_loop") == 0)
    {
        ODriveCAN::setAxisState(CANConfig::AXIS_STATE_CLOSED_LOOP_CONTROL);
    }
    else if (strcmp(payload, "set_vel_mode") == 0)
    {
        ODriveCAN::setControlMode(CANConfig::CONTROL_MODE_VELOCITY_CONTROL, CANConfig::INPUT_MODE_PASSTHROUGH);
        Serial.println(">> Tryb: VELOCITY PASSTHROUGH");
    }
    else if (strcmp(payload, "set_ramp_mode") == 0)
    {
        ODriveCAN::setControlMode(CANConfig::CONTROL_MODE_VELOCITY_CONTROL, CANConfig::INPUT_MODE_VEL_RAMP);
        Serial.println(">> Tryb: VELOCITY RAMP");
    }
    else if (strcmp(payload, "dump_errors") == 0)
    {
        ODriveCAN::requestODriveErrors(); // Wywołaj funkcję zapytania o błędy
        Serial.println(">> ODrive: Requesting Error Dump...");
    }
    else if (strcmp(payload, "reboot_odrive") == 0)
    { // <--- Obsługa Reboot
        ODriveCAN::rebootODrive();
        Serial.println(">> REBOOTING ODRIVE...");
    }   
}

void NetworkHandlers::feedbackEncHandler(
    struct NetworkState &ns, const struct HardwareCommandState &hcs
)
{
    char msg[128];
    snprintf(msg, sizeof(msg), "{\"v_meas\":%.2f,\"p_meas\":%.2f}", hcs.measuredVel, hcs.measuredPos);
    ns.client.publish(NetworkConfig::TOPIC_FEEDBACK, msg);
}

void NetworkHandlers::errorEncHandler(
    struct NetworkState &ns, const uint32_t errorDesc
)
{
    char errBuf[64];
    snprintf(errBuf, sizeof(errBuf), "{\"error\": \"0x%X\"}", errorDesc);
    ns.client.publish(NetworkConfig::TOPIC_FEEDBACK, errBuf);
}