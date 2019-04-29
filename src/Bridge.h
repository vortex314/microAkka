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

class Bridge : public Actor
{

    bool _connected;
    StaticJsonDocument<2000> _jsonBuffer;
    std::string _address;
    ActorRef& _mqtt;
    uint32_t _rxd;
    uint32_t _txd;

public:
    Bridge(ActorRef& mqtt);
    ~Bridge();
    void preStart();
    Receive& createReceive();
    enum { Connected=H("Connected") };

    bool jsonToMessage(Msg& msg,std::string& topic,std::string& message);
    bool messageToJson(std::string& topic,std::string& message,Msg& msg);

    bool handleMqttMessage(const char* message);
};
#endif
