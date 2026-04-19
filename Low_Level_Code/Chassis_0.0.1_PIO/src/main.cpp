// main.cpp
/*
 * FINAL DEBUG CODE: ESP32 + MQTT + ODrive + LEDC SERVO (FreeRTOS Version)
 * Wersja Modułowa
 */

#include <Arduino.h>

// Dołączamy nasze moduły
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

// --- SETUP ---
void setup()
{
    Serial.begin(HardwareConfig::SERIAL_BAUD_RATE);
    delay(1000); 
    Serial.println("\n\n>>> SYSTEM BOOT START <<<");

    // Inicjalizacja modułów
    ServoControl::initServo(hcs);
    Network::initNetwork(ns, hcs);
    ODriveCAN::initCAN();

    Serial.println(">>> SETUP COMPLETE <<<");
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
            Serial.println("!!! WATCHDOG: Utrata polaczenia - STOP !!!");
            hcs.wheels[0].targetVelocity = hcs.wheels[1].targetVelocity = 0.0f;
        }
    }

    // 1. Obsługa MQTT i Ethernet
    Network::handleNetwork();

    // 2. Wysyłanie do ODrive (CAN) co 50ms
    static uint32_t lastCanCycle = 0;
    if (now - lastCanCycle > CANConfig::CAN_CYCLE_DELAY)
    {
        lastCanCycle = now;
        // Serial.println("[CAN] Sending Vel...");
        ODriveCAN::sendVelocity(hcs.wheels[0].targetVelocity, CANConfig::FRONT);
        ODriveCAN::sendVelocity(hcs.wheels[1].targetVelocity, CANConfig::REAR);

        ODriveCAN::requestEncoderData(CANConfig::ODriveId::FRONT);
        ODriveCAN::requestEncoderData(CANConfig::ODriveId::REAR);
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