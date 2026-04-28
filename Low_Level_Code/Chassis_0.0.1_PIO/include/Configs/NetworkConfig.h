// NetworkConfig.h
#ifndef NETWORK_CONFIG_H
#define NETWORK_CONFIG_H

#include <cstdint>

namespace NetworkConfig
{
    // --- Network adresses ---
    constexpr uint16_t MQTT_PORT                    = 1883;
    constexpr const char *const MQTT_SERVER_IP      = "192.168.1.1"; 
    constexpr const char *const MQTT_SERVER_ID      = "";
    constexpr uint8_t MAC[6]                        = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
    constexpr uint8_t IP[4]                         = {192, 168, 1, 177};

    // --- MQTT configuration ---
    constexpr uint16_t MQTT_FEEDBACK_DELAY          = 100;
    
    constexpr const char *const TOPIC_SET_VEL       = "odrive/set_velocity";
    constexpr const char *const TOPIC_FEEDBACK      = "odrive/feedback";
    constexpr const char *const TOPIC_CMD           = "odrive/cmd";

    // --- Network Safety ---
    constexpr uint16_t MQTT_SAFETY_TIMEOUT          = 1000;
    constexpr uint8_t MQTT_MAX_RECONNECTION_RETRIES = 10;
    constexpr uint16_t MQTT_RECONNECTION_TIMEOUT    = 5000;
    constexpr uint16_t MQTT_CRITICAL_TIMEOUT        = 300000; // 5 minutes

    constexpr uint16_t MQTT_MAX_JSON_PAYLOAD        = 256;

    enum CalibrationStage : uint8_t
    {
        UNINITIALIZED = 0,
        CALIBRATION = 1,
        CLOSED_LOOP = 2,
        VELOCITY_MODE = 3,
        RAMP_MODE = 4
    };
}

#endif