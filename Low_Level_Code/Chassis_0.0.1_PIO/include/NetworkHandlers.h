// NetworkHandlers.h
#ifndef NETWORK_HANDLERS_H
#define NETWORK_HANDLERS_H

#include <ArduinoJson.h>

#include "Configs/NetworkConfig.h"
#include "Configs/CANConfig.h"

#include "States/NetworkState.h"
#include "States/HardwareCommandState.h"

namespace NetworkHandlers
{
    void setVelocityHandler(
        StaticJsonDocument<NetworkConfig::MQTT_MAX_JSON_PAYLOAD> &doc,
        struct NetworkState &ns,
        struct HardwareCommandState &hcs
    );

    void controlCmdHandler(StaticJsonDocument<NetworkConfig::MQTT_MAX_JSON_PAYLOAD> &doc);

    void feedbackEncHandler(
        struct NetworkState &ns, const struct HardwareCommandState &hfs
    );
    
    void errorEncHandler(
        struct NetworkState &ns,
        const CANConfig::ODriveId node_id,
        const uint32_t errorDesc
    );
}

#endif