// ServoControl.cpp
#include <Arduino.h>
#include <hal/ledc_hal.h>
#include <freertos/task.h>

#include "Pins.h"
#include "ServoControl.h"

#include "Configs/HardwareConfig.h"
#include "States/HardwareCommandState.h"

static float currentServoPos_ = HardwareConfig::STARTING_SERVO_POS;

// Uchwyt zadania jest lokalny, chyba że potrzebujesz go gdzie indziej
static TaskHandle_t ServoTaskHandle;

void ServoControl::initServo(struct HardwareCommandState &hcs)
{
    Serial.print("Init LEDC on Pin "); Serial.print(Pins::SERVO_PIN); Serial.println("...");
    // This single line sets the frequency and resolution and attaches the pin
    ledcAttach(Pins::SERVO_PIN, HardwareConfig::LEDC_FREQ, HardwareConfig::LEDC_RESOLUTION);
    setServoMicroseconds(HardwareConfig::MID_PULSE);
    
    Serial.println("Creating Servo Task...");
    xTaskCreate(
        servoTask,    // Funkcja
        "ServoTask",  // Nazwa
        2048,         // Pamięć
        (void *) &hcs, // Parametry
        1,            // Priorytet
        &ServoTaskHandle // Uchwyt
    );
    Serial.println("LEDC & Task Init OK");
}

void ServoControl::setServoMicroseconds(int us)
{
    if (us < HardwareConfig::MIN_PULSE) us = HardwareConfig::MIN_PULSE;
    if (us > HardwareConfig::MAX_PULSE) us = HardwareConfig::MAX_PULSE;
    
    long duty = ((long)us * 65536L) / 20000L;
    ledcWrite(Pins::SERVO_PIN, duty);
}

void ServoControl::servoTask(void * parameter)
{
    Serial.println("[TASK] Servo Task STARTED");

    HardwareCommandState* hcs = (HardwareCommandState*)parameter;

    for(;;)
    {
        // 1. Oblicz cel (for now, identical for both front and rear wheels).
        float targetPulse = 1500.0 + (hcs->wheels[0].targetSteering * 1000.0); // TODO: change accordingly to be able to control each odrive separately
        
        if (targetPulse < (float)HardwareConfig::MIN_PULSE) targetPulse = (float)HardwareConfig::MIN_PULSE;
        if (targetPulse > (float)HardwareConfig::MAX_PULSE) targetPulse = (float)HardwareConfig::MAX_PULSE;

        // 2. Wygładzanie
        float diff = targetPulse - currentServoPos_;

        if (abs(diff) > HardwareConfig::DEADBAND)
        {
            currentServoPos_ += (diff * HardwareConfig::SMOOTH_FACTOR);
            // setServoMicroseconds((int)currentServoPos); // Opcjonalnie usuń printy w produkcji
        } 
        else
        {
            if (abs(diff) > 0.5)
            {
                currentServoPos_ = targetPulse;
                // Serial.println("[TASK] Final Adjust");
            }
        }
        
        // Zawsze aktualizuj PWM
        setServoMicroseconds((int)currentServoPos_);

        // Usypiamy wątek na 20ms
        vTaskDelay(20 / portTICK_PERIOD_MS); 
    }
}