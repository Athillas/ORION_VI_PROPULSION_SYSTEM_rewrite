// main.cpp
/*
 * FINAL DEBUG CODE: ESP32 + MQTT + ODrive + LEDC SERVO (FreeRTOS Version)
 * Wersja Modułowa
 */

#include <Arduino.h>

// Dołączamy nasze moduły
#include "Pins.h"

#include "ServoControl.h"
#include "ODriveCAN.h"
#include "Network.h"

#include "States/HardwareCommandState.h"
#include "States/NetworkState.h"

#include "Configs/HardwareConfig.h"
#include "Configs/CANConfig.h"

// --- States ---
// (Initialized via constructors)
struct NetworkState ns {};
struct HardwareCommandState hcs {};

// Yellow- CAN HIGH

// --- SETUP ---
void setup()
{
    Serial.begin(HardwareConfig::SERIAL_BAUD_RATE);
    delay(1000); 
    Serial.println("\n\n>>> SYSTEM BOOT START <<<\n");
    
    // Inicjalizacja modułów
    Pins::init_pins();
    Serial.println();
    ServoControl::initServo(hcs);
    Serial.println();
    Network::initNetwork(ns, hcs);
    Serial.println();
    ODriveCAN::initCAN();

    Serial.println("\n>>> SETUP COMPLETE <<<\n\n");
}

// ==========================================
// LOOP
// ==========================================
void loop()
{
    uint32_t now = millis();
 
    if (now - ns.lastMqttCmdTime > NetworkConfig::MQTT_SAFETY_TIMEOUT)
    {
        if (hcs.wheels[0].targetVelocity != 0.0f || hcs.wheels[1].targetVelocity != 0.0f)
        {
            Serial.println("[LOOP] 13");
            Serial.println("!!! WATCHDOG: Utrata polaczenia - STOP !!!");
            hcs.wheels[0].targetVelocity = 0.0f;
            hcs.wheels[1].targetVelocity = 0.0f;
        }
    }

    // 1. Obsługa MQTT i Ethernet
    Network::handleNetwork();

    static unsigned long lastCanCycle = 0;
    if (now - lastCanCycle > CANConfig::CAN_CYCLE_TIMEOUT)
    {
        lastCanCycle = now;
        
        ODriveCAN::sendVelocity(CANConfig::FRONT, hcs.wheels[CANConfig::FRONT].targetVelocity);
        ODriveCAN::sendVelocity(CANConfig::REAR, hcs.wheels[CANConfig::REAR].targetVelocity);
        
        ODriveCAN::requestEncoderData(CANConfig::FRONT);
        ODriveCAN::requestEncoderData(CANConfig::REAR);
    }

    // 3. Odbiór danych z CAN
    ODriveCAN::handleCANMessages(hcs);

    // 4. Wysyłanie Feedbacku MQTT co 100ms
    static uint32_t lastMqttPub = 0;
    if (now - lastMqttPub > NetworkConfig::MQTT_FEEDBACK_DELAY)
    {
        lastMqttPub = now;
        Network::sendFeedbackMessage(hcs);
    }
}