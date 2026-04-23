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
	float torqueFF = 0.0f;

	CAN.beginPacket(CANConfig::getPacketId(id, CANConfig::SET_INPUT_VEL));
	CAN.write((uint8_t*)&velocity, 4);
	CAN.write((uint8_t*)&torqueFF, 4);
	CAN.endPacket();
}

void ODriveCAN::setAxisState(int32_t state, CANConfig::ODriveId id)
{
	CAN.beginPacket(CANConfig::getPacketId(id, CANConfig::SET_AXIS_STATE));
	CAN.write((uint8_t*)&state, 4);
	CAN.endPacket();
}

void ODriveCAN::setControlMode(int32_t controlMode, int32_t inputMode, CANConfig::ODriveId id)
{
	CAN.beginPacket(CANConfig::getPacketId(id, CANConfig::SET_CONTROLLER_MODE));
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
	CAN.endPacket();
}

void ODriveCAN::rebootODrive(CANConfig::ODriveId id)
{
	CAN.beginPacket(CANConfig::getPacketId(id, CANConfig::ODriveCommand::REBOOT_ODRIVE));
	CAN.endPacket();
}

void ODriveCAN::requestODriveErrors(CANConfig::ODriveId id)
{
	// Wysyłamy ramkę RTR (prośbę o dane) - 3. argument 'true' oznacza RTR
	CAN.beginPacket(CANConfig::getPacketId(id, CANConfig::GET_ERROR), 8, true);
	CAN.endPacket();
}

void ODriveCAN::handleCANMessages(struct HardwareCommandState &hcs)
{
	uint32_t packet_size = CAN.parsePacket();
	if(!packet_size) return;

	// bits 0-4: command id, bits 5-10: node id
	CANConfig::ODriveCommand cmd_id = (CANConfig::ODriveCommand) (CAN.packetId() & 0x1F); // Extracting bits 0-4
	CANConfig::ODriveId node_id = (CANConfig::ODriveId) ((CAN.packetId() >> 5) & 0x3F); // Extracting bits 5-10

	if (node_id >= 2)
	{
		Serial.println("[CAN] node_id is out of bounds in handleCANMessages. Check ODrive configurations.");
	}

	if (cmd_id == CANConfig::GET_ENCODER && packet_size >= 8)
	{
		uint8_t buffer[8];
		CAN.readBytes(buffer, 8);
		memcpy(&hcs.wheels[node_id].measuredPos, &buffer[0], 4);
		memcpy(&hcs.wheels[node_id].measuredVel, &buffer[4], 4);
		// Serial.println("[CAN] Encoder Data Recv");
	}
	else if (cmd_id == CANConfig::GET_ERROR && packet_size >= 4)
	{
		uint8_t buffer[4];
		CAN.readBytes(buffer, 4);
		memcpy(&hcs.wheels[node_id].activeErrors, &buffer[0], 4);
		
		// SIDE- 0: left, 1: right
		// node id - 0: front, 1: rear
		Serial.print("[CAN] ODrive id "); Serial.print(node_id); Serial.print(HardwareConfig::SIDE);
		Serial.print(" ERROR: "); Serial.println(hcs.wheels[node_id].activeErrors, HEX);
		Network::sendErrorMessage(node_id, hcs.wheels[node_id].activeErrors); // Funkcja z Network.h
	}
}