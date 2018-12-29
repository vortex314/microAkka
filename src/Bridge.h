#ifndef __BRIDGE_H
#define __BRIDGE_H

#include <Akka.h>
#include <ArduinoJson.h>
#include <Mqtt.h>

// #define ADDRESS "tcp://test.mosquitto.org:1883"
//#define CLIENTID "microAkka"
//#define TOPIC "dst/steer/system"
//#define PAYLOAD "[\"pclat/aliveChecker\",1234,23,\"hello\"]"
#define QOS 0
#define TIMEOUT 10000L

class Bridge : public Actor {

		bool _connected;
		StaticJsonBuffer<2000> _jsonBuffer;
		string _clientId;
		string _address;
		ActorRef _mqtt;

	public:
		static MsgClass MQTT_PUBLISH_RCVD();
		Bridge(va_list args);
		~Bridge();
		void preStart();
		Receive& createReceive();
		enum { Connected=H("Connected") };

		bool jsonToMessage(Msg& msg,std::string& json);
		bool messageToJson(std::string& json,Msg& msg);

		bool handleMqttMessage(const char* message);
};
#endif
