// Pins.h
#ifndef PINS_H
#define PINS_H

#include <Arduino.h>
#include <SPI.h>
#include <cstdint>

namespace Pins
{
    constexpr uint8_t SPI_MISO_PIN  = GPIO_NUM_19;
    constexpr uint8_t SPI_MOSI_PIN  = GPIO_NUM_23;
    constexpr uint8_t SPI_SCK_PIN   = GPIO_NUM_18;
    
    constexpr uint8_t ETH_CS_PIN    = GPIO_NUM_5;
    constexpr uint8_t ETH_RST_PIN   = GPIO_NUM_27;

    constexpr uint8_t CAN_TX_PIN    = GPIO_NUM_26;
    constexpr uint8_t CAN_RX_PIN    = GPIO_NUM_25;
    constexpr uint8_t SERVO_PIN     = GPIO_NUM_32;

    inline void init_pins()
    {
        Serial.println("[PINS] Initting pins...");

        pinMode(Pins::ETH_RST_PIN, OUTPUT);

        if(!SPI.begin(Pins::SPI_SCK_PIN,
            Pins::SPI_MISO_PIN,
            Pins::SPI_MOSI_PIN,
            Pins::ETH_CS_PIN
        ))
        {
            Serial.println("[PINS] Failed to init SPI pins!");
        }
        else
        {
            Serial.println("[PINS] Pins init successful");
        }
    }
}

#endif