// ODriveCAN.cpp
#include <Arduino.h>
#include <CAN.h>

#include "ODriveCAN.h"
#include "Network.h" // Potrzebne, żeby wysyłać błędy przez MQTT

#include "Pins.h"
#include "Configs/CANConfig.h"

enum PacketId : uint8_t
{
	GET_ERRORS				= (CANConfig::ODRIVE_NODE_ID << 5) | CANConfig::CMD_ID_GET_ERROR,
	SET_AXIS_STATE 			= (CANConfig::ODRIVE_NODE_ID << 5) | CANConfig::CMD_ID_SET_AXIS_STATE,
	GET_ENCODER				= (CANConfig::ODRIVE_NODE_ID << 5) | CANConfig::CMD_ID_GET_ENCODER,
	SET_CONTROL_MODE 		= (CANConfig::ODRIVE_NODE_ID << 5) | CANConfig::CMD_ID_SET_CONTROLLER_MODE,
	SET_INPUT_VEL 			= (CANConfig::ODRIVE_NODE_ID << 5) | CANConfig::CMD_ID_SET_INPUT_VEL,
	REBOOT_ODRIVE			= (CANConfig::ODRIVE_NODE_ID << 5) | CANConfig::CMD_ID_REBOOT_ODRIVE,
	CLEAR_ERRORS			= (CANConfig::ODRIVE_NODE_ID << 5) | CANConfig::CMD_ID_CLEAR_ERRORS,
	REQUEST_ODRIVE_ERRORS 	= (CANConfig::ODRIVE_NODE_ID << 5) | CANConfig::CMD_ID_GET_ERROR,
};

void ODriveCAN::initCAN()
{
	Serial.println("Init CAN...");
	CAN.setPins(Pins::CAN_RX_PIN, Pins::CAN_TX_PIN);
	if (!CAN.begin(CANConfig::CAN_BAUD_RATE)) {
		Serial.println("ERROR: CAN Init Failed!");
		while(1); // Zatrzymaj, jeśli CAN nie działa
	} else {
		Serial.println("CAN Init OK");
	}
}

void ODriveCAN::sendVelocity(float velocity)
{
	float torqueFF = 0.0f;

	CAN.beginPacket(PacketId::SET_INPUT_VEL);
	CAN.write((uint8_t*)&velocity, 4);
	CAN.write((uint8_t*)&torqueFF, 4);
	CAN.endPacket();
}

void ODriveCAN::setAxisState(int32_t state)
{
	CAN.beginPacket(PacketId::SET_AXIS_STATE);
	CAN.write((uint8_t*)&state, 4);
	CAN.endPacket();
}

void ODriveCAN::setControlMode(int32_t controlMode, int32_t inputMode)
{
	CAN.beginPacket(PacketId::SET_CONTROL_MODE);
	CAN.write((uint8_t*)&controlMode, 4);
	CAN.write((uint8_t*)&inputMode, 4);
	CAN.endPacket();
}

void ODriveCAN::requestEncoderData()
{
	CAN.beginPacket(PacketId::GET_ENCODER, 8, true);
	CAN.endPacket();
}

void ODriveCAN::clearErrors()
{
	CAN.beginPacket(PacketId::CLEAR_ERRORS);
	CAN.endPacket();
}

void ODriveCAN::rebootODrive()
{
	CAN.beginPacket(PacketId::REBOOT_ODRIVE);
	CAN.endPacket();
}

void ODriveCAN::requestODriveErrors()
{
	// Wysyłamy ramkę RTR (prośbę o dane) - 3. argument 'true' oznacza RTR
	CAN.beginPacket(PacketId::GET_ERRORS, 8, true);
	CAN.endPacket();
}

void ODriveCAN::handleCANMessages(struct HardwareCommandState &hcs)
{
	uint32_t packetSize = CAN.parsePacket();
	if(!packetSize) return;

	uint32_t cmdId = CAN.packetId() & 0x01F;
	
	if (cmdId == CANConfig::CMD_ID_GET_ENCODER && packetSize >= 8)
	{
		uint8_t buffer[8];
		CAN.readBytes(buffer, 8);
		memcpy(&hcs.measuredPos, &buffer[0], 4);
		memcpy(&hcs.measuredVel, &buffer[4], 4);
		// Serial.println("[CAN] Encoder Data Recv");
	}
	else if (cmdId == CANConfig::CMD_ID_GET_ERROR && packetSize >= 4)
	{
		uint8_t buffer[4];
		CAN.readBytes(buffer, 4);
		memcpy(&hcs.activeErrors, &buffer[0], 4);
		
		Serial.print("[CAN] ODrive ERROR: "); Serial.println(hcs.activeErrors, HEX);
		Network::sendErrorMessage(hcs.activeErrors); // Funkcja z Network.h
	}
}