// ODriveCAN.cpp
#include <Arduino.h>
#include <CAN.h>

#include "ODriveCAN.h"
#include "Network.h" // Potrzebne, żeby wysyłać błędy przez MQTT

#include "Pins.h"
#include "Configs/CANConfig.h"

void ODriveCAN::initCAN()
{
	Serial.println("[CAN] Init CAN...");
	CAN.setPins(Pins::CAN_RX_PIN, Pins::CAN_TX_PIN);
	if (!CAN.begin(CANConfig::CAN_BAUD_RATE))
	{
		Serial.println("[CAN ERROR]: CAN Init Failed!");
		while(1); // Zatrzymaj, jeśli CAN nie działa
	}
	else
	{
		Serial.println("[CAN] CAN Init OK");
	}
}

void ODriveCAN::sendVelocity(float velocity, CANConfig::ODriveId id)
{
	Serial.println("[CAN SEND VELOCITY]");
	float torqueFF = 0.0f;

	CAN.beginPacket(CANConfig::getPacketId(id, CANConfig::SET_INPUT_VEL));
	// assumpts little-endian
    CAN.write((uint8_t*)&velocity, 4);
	CAN.write((uint8_t*)&torqueFF, 4);
	CAN.endPacket();
}

void ODriveCAN::setAxisState(uint32_t state, CANConfig::ODriveId id)
{
	Serial.println("[SET AXIS STATE]\n\n\n\n");
	Serial.println(state);
	Serial.println("\n\n\n");
	
	CAN.beginPacket(CANConfig::getPacketId(id, CANConfig::SET_AXIS_STATE));
	// assumpts little-endian
    CAN.write((uint8_t*)&state, 4);
	CAN.endPacket();
}

void ODriveCAN::setControlMode(uint32_t controlMode, uint32_t inputMode, CANConfig::ODriveId id)
{
	CAN.beginPacket(CANConfig::getPacketId(id, CANConfig::SET_CONTROLLER_MODE));
	// assumpts little-endian
    CAN.write((uint8_t*)&controlMode, 4);
	CAN.write((uint8_t*)&inputMode, 4);
	CAN.endPacket();
}

void ODriveCAN::requestEncoderData(CANConfig::ODriveId id)
{
	CAN.beginPacket(CANConfig::getPacketId(id, CANConfig::GET_ENCODER), 8, true);
	CAN.endPacket();
}

void ODriveCAN::clearErrors(CANConfig::ODriveId id)
{
	CAN.beginPacket(CANConfig::getPacketId(id, CANConfig::CLEAR_ERRORS));
	uint8_t identify = 0;
    CAN.write(&identify, 1);
    CAN.endPacket();
}

void ODriveCAN::rebootODrive(CANConfig::ODriveId id)
{
	CAN.beginPacket(CANConfig::getPacketId(id, CANConfig::ODriveFeedbackCommand::REBOOT_ODRIVE));
    uint8_t payload = 2;
    CAN.write(&payload, 1);
	CAN.endPacket();
}


// https://docs.odriverobotics.com/v/latest/fibre_types/com_odriverobotics_ODrive.html#ODrive.Error
void ODriveCAN::requestODriveErrors(CANConfig::ODriveId id)
{
	// Wysyłamy ramkę RTR (prośbę o dane) - 3. argument 'true' oznacza RTR
	CAN.beginPacket(CANConfig::getPacketId(id, CANConfig::GET_ERROR), 8, true);
	CAN.endPacket();
}

// void ODriveCAN::handleCANMessages(struct HardwareCommandState &hcs)
// {
// 	uint32_t packet_size = CAN.parsePacket();
// 	if(!packet_size) return;

// 	// bits 0-4: command id, bits 5-10: node id
// 	CANConfig::ODriveCommand cmd_id = (CANConfig::ODriveCommand) (CAN.packetId() & 0x1F); // Extracting bits 0-4
// 	CANConfig::ODriveId node_id = (CANConfig::ODriveId) ((CAN.packetId() >> 5) & 0x3F); // Extracting bits 5-10
// 	bool isRtr = CAN.packetRtr(); // Sprawdzamy czy to nie jest żądanie

// 	if (node_id >= 2)
// 	{
// 		Serial.println("[CAN] node_id is out of bounds in handleCANMessages. Check ODrive configurations.");
// 	}

// 	if (cmd_id == CANConfig::GET_ENCODER && packet_size >= 8)
// 	{
// 		uint8_t buffer[8];
// 		CAN.readBytes(buffer, 8);
// 		memcpy(&hcs.wheels[node_id].measuredPos, &buffer[0], 4);
// 		memcpy(&hcs.wheels[node_id].measuredVel, &buffer[4], 4);
// 		// Serial.println("[CAN] Encoder Data Recv");
// 	}
// 	else if (cmd_id == CANConfig::GET_ERROR && packet_size >= 4)
// 	{
// 		uint8_t buffer[4];
// 		CAN.readBytes(buffer, 4);
// 		memcpy(&hcs.wheels[node_id].activeErrors, &buffer[0], 4);
		
// 		// SIDE- 0: left, 1: right
// 		// node id - 0: front, 1: rear
// 		Serial.print("[CAN] ODrive id "); Serial.print(node_id); Serial.print(HardwareConfig::SIDE);
// 		Serial.print(" ERROR: "); Serial.println(hcs.wheels[node_id].activeErrors, HEX);
// 		Network::sendErrorMessage(node_id, hcs.wheels[node_id].activeErrors); // Funkcja z Network.h
// 	}
// }

void ODriveCAN::handleCANMessages(struct HardwareCommandState &hcs)
{
    uint32_t packet_size = CAN.parsePacket();
    if (!packet_size) return;

    // bits 0-4: command id, bits 5-10: node id
    CANConfig::ODriveFeedbackCommand cmd_id = (CANConfig::ODriveFeedbackCommand) (CAN.packetId() & 0x1F); // Extracting bits 0-4
    CANConfig::ODriveId node_id = (CANConfig::ODriveId) ((CAN.packetId() >> 5) & 0x3F); // Extracting bits 5-10 CANConfig::FRONT
    
    if(node_id != 0) return; // REMOVE THIS LINE LATER, only for test with one motor!!!! 
    
    bool isRtr = CAN.packetRtr(); // Check if it's a Remote Transmission Request

    if (node_id >= 2)
    {
        Serial.println("[CAN] node_id is out of bounds in handleCANMessages. Check ODrive configurations.");
        return; // Good practice to prevent out-of-bounds array access below
    }

    if(isRtr) return;

    // Create a safe, zero-initialized buffer and dump data into it
    uint8_t buffer[8] = {0};

    uint8_t i = 0;
    while (CAN.available() && i < 8) {
        buffer[i++] = CAN.read();
    }


    // 1. AUTOMATIC ERRORS FROM HEARTBEAT (0x01)
    if (cmd_id == CANConfig::HEARTBEAT && packet_size >= 4)
    {
        uint32_t currentError;
        memcpy(&currentError, &buffer[0], 4);

        // --- Heartbeat Logging ---
        Serial.print("[CAN] ODrive id "); Serial.print(node_id); Serial.print(HardwareConfig::SIDE);
        Serial.print(" Heartbeat -> Error: 0x"); 
        Serial.print(currentError, HEX);
            
        // If the frame has at least 5 bytes, extract the Axis State
        if (packet_size >= 5) {
            uint8_t axisState = buffer[4];
            Serial.print(" | Axis State: ");
            Serial.print(axisState);
        }
        Serial.println();
        // -------------------------
        
        // Detected a NEW error
        if (currentError != 0 && currentError != hcs.wheels[node_id].activeErrors) {
            hcs.wheels[node_id].activeErrors = currentError;
            Serial.print("[CAN] WARNING! ODrive id "); Serial.print(node_id); Serial.print(HardwareConfig::SIDE);
            Serial.print(" reports a NEW error: 0x"); 
            Serial.println(hcs.wheels[node_id].activeErrors, HEX);
            Network::sendErrorMessage(node_id, hcs.wheels[node_id].activeErrors); // Send via MQTT immediately
        } 
        // Errors have been cleared
        else if (currentError == 0 && hcs.wheels[node_id].activeErrors != 0) {
            hcs.wheels[node_id].activeErrors = 0; 
            Serial.print("[CAN] ODrive id "); Serial.print(node_id); Serial.print(HardwareConfig::SIDE);
            Serial.println(" errors have been cleared.");
        }
    }
    // 2. HANDLE ENCODER DATA (0x09)
    else if (cmd_id == CANConfig::GET_ENCODER && packet_size >= 8)	
    {
        memcpy(&hcs.wheels[node_id].measuredPos, &buffer[0], 4);
        memcpy(&hcs.wheels[node_id].measuredVel, &buffer[4], 4);
    }
    
    // 3. DIRECT RESPONSE TO GET_ERROR (0x03)
    else if (cmd_id == CANConfig::GET_ERROR && packet_size >= 4)
    {
        uint32_t requestedError;
        memcpy(&requestedError, &buffer[0], 4);
        
        // Update the state struct with the requested error
        hcs.wheels[node_id].activeErrors = requestedError;

        Serial.print("[CAN] ODrive id "); Serial.print(node_id); Serial.print(HardwareConfig::SIDE);
        Serial.print(" ERROR (Requested): 0x"); Serial.println(hcs.wheels[node_id].activeErrors, HEX);
        Network::sendErrorMessage(node_id, hcs.wheels[node_id].activeErrors); 
    }
    else
    {
        Serial.print("[CAN] Unhandled cmd_id: ");
        Serial.println(cmd_id);
    }
}