// Network.h
#ifndef NETWORK_H
#define NETWORK_H

#include <Arduino.h>
#include "States/NetworkState.h"
#include "States/HardwareCommandState.h"

namespace Network
{
    void initNetwork(struct NetworkState &ns, struct HardwareCommandState &hcs);
    void handleNetwork(); // Wywoływane w loop()
    void sendFeedbackMessage(struct HardwareCommandState &hcs);
    void sendErrorMessage(uint32_t errorDesc);
}

#endif