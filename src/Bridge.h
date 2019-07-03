#ifndef __BRIDGE_H
#define __BRIDGE_H

#include <Akka.h>
#define ARDUINOJSON_ENABLE_STD_STRING 1
#include <ArduinoJson.h>
#include <Mqtt.h>

// #define ADDRESS "tcp://test.mosquitto.org:1883"
//#define CLIENTID "microAkka"
//#define TOPIC "dst/steer/system"
//#define PAYLOAD "[\"pclat/aliveChecker\",1234,23,\"hello\"]"
#define QOS 0
#define TIMEOUT 10000L

class Bridge : public Actor {
		StaticJsonDocument<3000> _jsonDoc;
		std::string _address;
		ActorRef& _mqtt;
		uint32_t _rxd;
		uint32_t _txd;
		std::unordered_map<uid_type,ActorRef*>::iterator _currentActorRef;
		bool _mqttConnected=false;

	public:
		static const MsgClass Publish;
		static const MsgClass Properties;
		static const MsgClass PropertiesReply;


		Bridge(ActorRef& mqtt);
		~Bridge();
		void preStart();
		Receive& createReceive();
		enum { Connected=H("Connected") };
		ActorRef* nextRef();

		bool jsonEventToMessage(Msg& msg,std::string& topic,std::string& message);
		bool jsonCommandToMessage(Msg& msg,std::string& topic,std::string& message);

		bool msgToJson(Msg& msg,std::string& topic,std::string& message);
		bool msgToJsonCmd(std::string& topic,std::string& message,Msg& msg);
		bool msgToJsonEvents(Msg& msg);
		uint32_t fields(Msg& msg,Tag& tag);


		bool topicToMsg(Msg& msg,std::string& topic);
		bool messageToMsg(Msg& msg,std::string& message);

		bool handleMqttMessage(const char* message);
		void subscribeEventBus();
};
#endif
