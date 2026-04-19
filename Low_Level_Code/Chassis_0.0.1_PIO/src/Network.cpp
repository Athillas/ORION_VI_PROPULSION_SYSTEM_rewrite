// Network.cpp
#include <Arduino.h>

#include <Arduino.h>
#include <ArduinoJson.h>
#include <Ethernet.h>

#include "Pins.h"

#include "ODriveCAN.h"
#include "Network.h"

#include "NetworkHandlers.h"

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
    
    if (length > NetworkConfig::MAX_JSON_PAYLOAD) {
        Serial.println("[MQTT] ERROR: Payload too big!");
        return; 
    }

    if (strcmp(topic, NetworkConfig::TOPIC_SET_VEL) == 0)
    {
        StaticJsonDocument<NetworkConfig::MAX_JSON_PAYLOAD> doc;
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
		for (unsigned int i = 0; i < length; i++) {
			Serial.print((char)payload[i]);
		}
		Serial.println();
		
		StaticJsonDocument<NetworkConfig::MAX_JSON_PAYLOAD> doc;
        DeserializationError error = deserializeJson(doc, payload, length);
		if(error)
        {
            Serial.print("[MQTT] Failed to deserialize the JSON!"); Serial.println(error.c_str());
            return;
        }

		
        ns_->lastMqttCmdTime = millis();
		NetworkHandlers::controlCmdHandler(doc);
    }
}

void Network::initNetwork(struct NetworkState &ns, struct HardwareCommandState &hcs)
{
    Serial.println("Init Ethernet...");

	ns_ = &ns;
	hcs_ = &hcs;
	memcpy(MAC_, NetworkConfig::MAC, 6);
	memcpy(IP_, NetworkConfig::IP, 4);
    
    pinMode(Pins::ETH_RST_PIN, OUTPUT);
    digitalWrite(Pins::ETH_RST_PIN, LOW); delay(100);
    digitalWrite(Pins::ETH_RST_PIN, HIGH); delay(100);
    
    Ethernet.init(Pins::ETH_CS_PIN);
    Ethernet.begin(MAC_, IP_);
    
    if (Ethernet.hardwareStatus() == EthernetNoHardware)
	{
        Serial.println("ERROR: W5500 not found!");
    } else
	{
        Serial.print("Ethernet IP_: "); Serial.println(Ethernet.localIP());
    }
    
    ns.client.setServer(NetworkConfig::MQTT_SERVER_ID, NetworkConfig::MQTT_PORT);
    ns.client.setCallback(callback);
    ns.client.setBufferSize(NetworkConfig::MAX_JSON_PAYLOAD);
}

inline static void reconnect()
{
	if(ns_ == nullptr)
	{
		Serial.print("[MQTT] Network is not initialized! Call the initNetwork() before using other methods. Quitting.");
		return;
	}

	if(ns_->client.connected()) return;

	static uint32_t lastRec = 0;

	if(millis() - lastRec < NetworkConfig::RECONNECTION_TIMEOUT) return;

	lastRec = millis();
	Serial.print("[MQTT] Connecting to "); Serial.print(NetworkConfig::MQTT_SERVER_IP); Serial.println("...");

	if(!ns_->client.connect(NetworkConfig::MQTT_SERVER_ID))
	{
		Serial.print("[MQTT] Failed, rc="); Serial.println(ns_->client.state());
		return;
	}

	Serial.println("[MQTT] Connected!");
	ns_->client.subscribe(NetworkConfig::TOPIC_SET_VEL);
	ns_->client.subscribe(NetworkConfig::TOPIC_CMD);
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

void Network::sendErrorMessage(const CANConfig::ODriveId node_id, uint32_t errorDesc)
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