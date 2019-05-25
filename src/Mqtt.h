#ifndef __MQTT_H
#define __MQTT_H
extern "C" {
#include "MQTTAsync.h"
#include "MQTTClient.h"
#include "MQTTClientPersistence.h"
}
#include <Akka.h>
#define ARDUINOJSON_ENABLE_STD_STRING 1
#include <ArduinoJson.h>

// #define ADDRESS "tcp://test.mosquitto.org:1883"
//#define CLIENTID "microAkka"
//#define TOPIC "dst/steer/system"
//#define PAYLOAD "[\"pclat/aliveChecker\",1234,23,\"hello\"]"
#define QOS 0
#define TIMEOUT 10000L

class Mqtt : public Actor {
		MQTTAsync _client;
		MQTTAsync_connectOptions _conn_opts;
		MQTTAsync_responseOptions _opts;
		bool _connected;
		std::string _clientId;
		std::string _address;
		MQTTAsync_token _deliveredtoken;

	public:
		static MsgClass PublishRcvd;
		static MsgClass Connected;
		static MsgClass Disconnected;
		static MsgClass Subscribe;
		static MsgClass Publish;

		Mqtt(const char* address);
		~Mqtt();
		void preStart();
		Receive& createReceive();
		static void onConnectionLost(void* context, char* cause);
		static void onDisconnect(void* context, MQTTAsync_successData* response);
		static void onSend(void* context, MQTTAsync_successData* response);
		static void onConnectFailure(void* context,
		                             MQTTAsync_failureData* response);
		static void onConnect(void* context, MQTTAsync_successData* response);
		static void onSubscribe(void* context, MQTTAsync_successData* response);
		static void onSubscribeFailure(void* context,
		                               MQTTAsync_failureData* response);
		static int onMessageArrived(void* context, char* topicName, int topicLen,
		                            MQTTAsync_message* message);
		static void onDeliveryComplete(void* context, MQTTAsync_token token);

		void mqttPublish(const char* topic, const char* message);
		void mqttSubscribe(const char* topic);
		void mqttConnect();
		void mqttDisconnect();
		void subscribeEventBus();

		bool handleMqttMessage(const char* message);
};
#endif
