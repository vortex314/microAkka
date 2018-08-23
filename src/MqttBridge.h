extern "C" {
#include "MQTTAsync.h"
#include "MQTTClient.h"
#include "MQTTClientPersistence.h"
}
#include <Akka.h>

#define ADDRESS "tcp://test.mosquitto.org:1883"
#define CLIENTID "microAkka"
#define TOPIC "dst/steer/system"
#define PAYLOAD "[\"pclat/aliveChecker\",1234,23,\"hello\"]"
#define QOS 1
#define TIMEOUT 10000L

class MqttBridge : public AbstractActor
{
public:
    MqttBridge();
    ~MqttBridge();
    void preStart();
    Receive& createReceive();
    static void connlost(void* context, char* cause);
    static void onDisconnect(void* context, MQTTAsync_successData* response);
    static void onSend(void* context, MQTTAsync_successData* response);
    static void onConnectFailure(void* context, MQTTAsync_failureData* response);
    static void onConnect(void* context, MQTTAsync_successData* response);
};