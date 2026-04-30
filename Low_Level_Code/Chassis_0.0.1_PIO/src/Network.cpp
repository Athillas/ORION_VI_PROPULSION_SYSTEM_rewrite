// Network.cpp
#include <Arduino.h>

#include <Arduino.h>
#include <ArduinoJson.h>
#include <Ethernet.h>
#include <SPI.h>

#include "Pins.h"

#include "ODriveCAN.h"
#include "Network.h"
#include "NetworkHandlers.h"
#include "StaticJsonMemoryAllocator.h"

#include "Configs/CANConfig.h"
#include "Configs/NetworkConfig.h"

#include "States/HardwareCommandState.h"
#include "States/NetworkState.h"


namespace
{
	// Non-const local copies, needed because of Ethernet's begin limitations.
	static uint8_t MAC_[6]; 
	static uint8_t IP_[4];
	
	/*
	  Local pointers to NetworkState and HardwareCommandState structs.

	  Initialized in initNetwork() method.
	  Needed because callback function cannot accept any additional arguments apart from the ones dictated by PubSubClient. 
	*/
	static struct NetworkState *ns_ = nullptr;
	static struct HardwareCommandState *hcs_ = nullptr;
}

void callback(char* topic, byte* payload, unsigned int length)
{
    if(ns_ == nullptr)
	{
		Serial.print("[MQTT] Local copy of NetworkState is not initialized in Network.cpp file! Call the initNetwork() before using other methods. Stopping.");
		return;
	}
	
	if(hcs_ == nullptr)
	{
		Serial.print("[MQTT] Local copy of HardwareCommandState is not initialized in Network.cpp file! Call the initNetwork() before using other methods. Stopping.");
		return;
	}
    // Serial.print("[MQTT] Recv Topic: "); Serial.println(topic);
    
    if (length > NetworkConfig::MQTT_MAX_JSON_PAYLOAD) {
        Serial.println("[MQTT] ERROR: Payload too big!");
        return; 
    }

	// Stack buffer for the JSON document
	uint8_t jsonBuffer[NetworkConfig::MQTT_MAX_JSON_PAYLOAD];
	StaticJsonDocument<NetworkConfig::MQTT_MAX_JSON_PAYLOAD> doc;

	// static StaticJsonMemoryAllocator allocator;
	// JsonDocument doc(&allocator);

    if (strcmp(topic, NetworkConfig::TOPIC_SET_VEL) == 0)
    {
        DeserializationError error = deserializeJson(doc, payload, length);
        if(error)
        {
            Serial.print("[MQTT] Failed to deserialize the JSON!"); Serial.println(error.c_str());
            return;
        }
        ns_->lastMqttCmdTime = millis();
        NetworkHandlers::setVelocityHandler(doc, *ns_, *hcs_);
    }
    else if (strcmp(topic, NetworkConfig::TOPIC_CMD) == 0)
    {
		Serial.print("[MQTT] Payload: ");
		for (unsigned int i = 0; i < length; i++)
		{
			Serial.print((char)payload[i]);
		}
		Serial.println();
        DeserializationError error = deserializeJson(doc, payload, length);
		if(error)
        {
            Serial.print("[MQTT] Failed to deserialize the JSON!"); Serial.println(error.c_str());
            return;
        }

		
        ns_->lastMqttCmdTime = millis();
		NetworkHandlers::controlCmdHandler(doc, *hcs_, *ns_);
    }
}

void Network::initNetwork(struct NetworkState &ns, struct HardwareCommandState &hcs)
{
    Serial.println("[MQTT] Init Ethernet...");

	if(ns_ == nullptr)
		ns_ = &ns;
	if(hcs_ == nullptr)
		hcs_ = &hcs;
	
	memcpy(MAC_, NetworkConfig::MAC, 6);
	memcpy(IP_, NetworkConfig::IP, 4);

	Serial.println("[MQTT] Resetting the WizNet...");
	digitalWrite(Pins::ETH_RST_PIN, LOW);
	delay(100);
    digitalWrite(Pins::ETH_RST_PIN, HIGH);
	delay(200);
	Serial.println("[MQTT] WizNet reset successful.");

    Ethernet.init(Pins::ETH_CS_PIN);
    Ethernet.begin(MAC_, IP_);
	delay(1500);
    
    if (Ethernet.hardwareStatus() == EthernetNoHardware)
	{
        Serial.println("[MQTT] ERROR: W5500 not found!");
    }
	else
	{
        Serial.print("[MQTT] Ethernet IP: "); Serial.println(Ethernet.localIP());
    }

	// 5. Diagnostyka początkowa kabla
	if (Ethernet.linkStatus() == LinkOFF) {
		Serial.println("[MQTT] WARNING: Network cable IS NOT connected at start!");
	}
    
    ns_->client.setServer(NetworkConfig::MQTT_SERVER_IP, NetworkConfig::MQTT_PORT);
    ns_->client.setCallback(callback);
    ns_->client.setBufferSize(NetworkConfig::MQTT_MAX_JSON_PAYLOAD);
}

static void reconnect()
{
	if(ns_ == nullptr)
	{
		Serial.print("[MQTT] Network is not initialized! Call the initNetwork() before using other methods. Quitting.");
		return;
	}
	static uint32_t firstRec = 0, lastRec = 0;
	static uint8_t retries = 0;

	if(!ns_->client.connected()) 
	{
		
		if(firstRec == 0) firstRec = millis();

		if(millis() - firstRec > NetworkConfig::MQTT_CRITICAL_TIMEOUT)
		{
			Serial.println("[MQTT CRITICAL] Persistent connection failure. Restarting...");
    		Serial.flush(); // Ensure the message is actually sent to the PC
			delay(100);
			ESP.restart();
			while(1); // makes sure no other code is executed after the instruction.
		}
		
		if (millis() - lastRec > NetworkConfig::MQTT_RECONNECTION_TIMEOUT)
		{
			lastRec = millis();
			static uint32_t lastReinit = 0;
			if(retries >= NetworkConfig::MQTT_MAX_RECONNECTION_RETRIES && millis() - lastReinit > 10000)
			{
				lastReinit = millis();
				Network::initNetwork(*ns_, *hcs_);
				retries = 0;
			}

			// 1. Najpierw sprawdzamy, czy w ogóle jest wpięty kabel!
			if (Ethernet.linkStatus() == LinkOFF)
			{
				Serial.println("[MQTT] BŁĄD: Kabel sieciowy odłączony (LinkOFF).");
				return; // Przerywamy próbę łączenia TCP, czekamy na kabel
			}
			// 2. Próba połączenia z brokerem
			Serial.print("[MQTT] Connecting to "); Serial.print(NetworkConfig::MQTT_SERVER_IP); Serial.println("...");
			if (ns_->client.connect(NetworkConfig::MQTT_SERVER_ID))
			{
				Serial.println("[MQTT] Connected!");
				// Subskrypcja tematów po udanym połączeniu
				ns_->client.subscribe(NetworkConfig::TOPIC_SET_VEL);
				ns_->client.subscribe(NetworkConfig::TOPIC_CMD);
				firstRec = lastRec = retries = 0;
			}
			else
			{
				Serial.print("[MQTT] Failed, rc="); Serial.println(ns_->client.state());
				retries += 1;
			}
		}
	}
	else
	{
		firstRec = lastRec = retries = 0;
	}
}

void Network::handleNetwork()
{
	if(ns_ == nullptr)
	{
		Serial.print("[MQTT] Network is not initialized! Call the initNetwork() before using other methods. Quitting.");
		return;
	}

	reconnect();
	ns_->client.loop();
}

void Network::sendFeedbackMessage(struct HardwareCommandState &hcs)
{
    if(ns_ == nullptr)
	{
		Serial.print("[MQTT] Network is not initialized! Call the initNetwork() before using other methods. Quitting.");
		return;
	}

	if(!ns_->client.connected())
	{
		Serial.println("[MQTT] Failed to send message; NetworkState::client is not connected!");
		return;
	}

	NetworkHandlers::feedbackEncHandler(*ns_, hcs);
}

void Network::sendErrorMessage(const uint8_t node_id, uint32_t errorDesc)
{
	if(ns_ == nullptr)
	{
		Serial.print("[MQTT] Network is not initialized! Call the initNetwork() before using other methods. Quitting.");
		return;
	}

	if(!ns_->client.connected())
	{
		Serial.println("[MQTT] Failed to send message; NetworkState's client is not connected!");
		return;
	}

	NetworkHandlers::errorEncHandler(*ns_, node_id, errorDesc);
}