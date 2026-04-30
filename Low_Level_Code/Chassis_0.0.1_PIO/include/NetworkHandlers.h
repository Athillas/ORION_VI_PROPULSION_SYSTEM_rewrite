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
        JsonDocument doc,
        struct NetworkState &ns,
        struct HardwareCommandState &hcs
    );

    void controlCmdHandler(JsonDocument doc, HardwareCommandState &hcs, NetworkState &ns);

    void feedbackEncHandler(
        struct NetworkState &ns, const struct HardwareCommandState &hfs
    );
    
    void errorEncHandler(
        struct NetworkState &ns,
        const uint8_t node_id,
        const uint32_t errorDesc
    );

    enum ODrivePacketCommand : uint8_t
    {
        CALIBRATE       = 0,
        CLOSED_LOOP     = 1,
        SET_VEL_MODE    = 2,
        SET_RAMP_MODE   = 3,
        DUMP_ERRORS     = 4,
        REBOOT_ODRIVE   = 5
    };
}

#endif