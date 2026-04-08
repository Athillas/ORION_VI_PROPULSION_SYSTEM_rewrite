// NetworkHandlers.h
#ifndef NETWORK_HANDLERS_H
#define NETWORK_HANDLERS_H

#include <ArduinoJson.h>

#include "Configs/NetworkConfig.h"

#include "States/NetworkState.h"
#include "States/HardwareCommandState.h"

namespace NetworkHandlers
{
    void setVelocityHandler(
        StaticJsonDocument<NetworkConfig::MAX_JSON_PAYLOAD> &doc,
        struct NetworkState &ns,
        struct HardwareCommandState &hcs
    );

    void controlCmdHandler(char *payload);

    void feedbackEncHandler(
        struct NetworkState &ns, const struct HardwareCommandState &hfs
    );
    
    void errorEncHandler(
        struct NetworkState &ns,
        const uint32_t errorDesc
    );
}

#endif