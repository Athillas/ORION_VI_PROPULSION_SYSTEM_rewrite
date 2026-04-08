// Pins.h
#ifndef PINS_H
#define PINS_H

#include <Arduino.h>
#include <cstdint>

namespace Pins
{
    constexpr uint8_t ETH_CS_PIN    = GPIO_NUM_21;
    constexpr uint8_t ETH_RST_PIN   = GPIO_NUM_22;
    constexpr uint8_t CAN_TX_PIN    = GPIO_NUM_5;
    constexpr uint8_t CAN_RX_PIN    = GPIO_NUM_4;
    constexpr uint8_t SERVO_PIN     = GPIO_NUM_32;
}

#endif