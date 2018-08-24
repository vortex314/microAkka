#include <ArduinoJson.h>
#include <MqttBridge.h>

int finished = 0;

volatile MQTTAsync_token deliveredtoken;

MqttBridge::MqttBridge(){};
MqttBridge::~MqttBridge() {}

void MqttBridge::preStart() {
    context().mailbox(remoteMailbox);
    _conn_opts = MQTTAsync_connectOptions_initializer;
    int rc;

    MQTTAsync_create(&_client, ADDRESS, CLIENTID, MQTTCLIENT_PERSISTENCE_NONE,
                     NULL);

    MQTTAsync_setCallbacks(_client, NULL, connlost, NULL, NULL);

    _conn_opts.keepAliveInterval = 20;
    _conn_opts.cleansession = 1;
    _conn_opts.onSuccess = onConnect;
    _conn_opts.onFailure = onConnectFailure;
    _conn_opts.context = this;
    if ((rc = MQTTAsync_connect(_client, &_conn_opts)) != MQTTASYNC_SUCCESS) {
        printf("Failed to start connect, return code %d\n", rc);
        exit(EXIT_FAILURE);
    }

    printf("Waiting for publication of %s\n"
           "on topic %s for client with ClientID: %s\n",
           PAYLOAD, TOPIC, CLIENTID);
}

StaticJsonBuffer<200> jsonBuffer;
/*
JsonObject& root = jsonBuffer.createObject();
root["sensor"] = "gps";
root["time"] = 1351824120;

JsonArray& data = root.createNestedArray("data");
data.add(48.756080);
data.add(2.302038);

*/

Receive& MqttBridge::createReceive() {
    return receiveBuilder()
        .match(AnyClass,
               [this](Envelope& msg) {
                   INFO(" message received %s:%s:%s in %s", msg.sender.path(),
                        msg.receiver.path(), msg.msgClass.name(),
                        context().self().path());
                   jsonBuffer.clear();
                   JsonArray& array = jsonBuffer.createArray();
                   array.add(msg.receiver.path());
                   array.add(msg.sender.path());
                   array.add(msg.msgClass.name());
                   array.add(msg.id);
                   std::string topic="dst/";
                   topic += msg.receiver.path();
                   std::string message;
                   array.printTo(message);
                   mqttPublish(topic,message);
               })
        .build();
}

void MqttBridge::connlost(void* context, char* cause) {
    MqttBridge* me = (MqttBridge*)context;

    me->_conn_opts = MQTTAsync_connectOptions_initializer;
    int rc;

    printf("\nConnection lost\n");
    printf("     cause: %s\n", cause);

    printf("Reconnecting\n");
    me->_conn_opts.keepAliveInterval = 20;
    me->_conn_opts.cleansession = 1;
    if ((rc = MQTTAsync_connect(me->_client, &me->_conn_opts)) !=
        MQTTASYNC_SUCCESS) {
        printf("Failed to start connect, return code %d\n", rc);
        finished = 1;
    }
}

void MqttBridge::onDisconnect(void* context, MQTTAsync_successData* response) {
    MqttBridge* me = (MqttBridge*)context;
    INFO("Successful disconnection %X\n", me);
    finished = 1;
}

void MqttBridge::onSend(void* context, MQTTAsync_successData* response) {
    MqttBridge* me = (MqttBridge*)context;

    printf("Message with token value %d delivery confirmed\n", response->token);
    /*
        opts.onSuccess = onDisconnect;
        opts.context = client;

        if((rc = MQTTAsync_disconnect(client, &opts)) != MQTTASYNC_SUCCESS) {
            printf("Failed to start sendMessage, return code %d\n", rc);
            exit(EXIT_FAILURE);
        }*/
}

void MqttBridge::onConnectFailure(void* context,
                                  MQTTAsync_failureData* response) {
    MqttBridge* me = (MqttBridge*)context;
    printf("Connect failed, rc %d\n", response ? response->code : 0);
    finished = 1;
}


void MqttBridge::mqttPublish(std::string& topic,std::string& message){
    INFO(" MQTT PUB %s:%s",topic.c_str(),message.c_str());
    MQTTAsync_message pubmsg = MQTTAsync_message_initializer;
    int rc;
    _opts.onSuccess = onSend;
    _opts.context = this;

    pubmsg.payload = (void*)message.c_str();
    pubmsg.payloadlen = (int)message.length();
    pubmsg.qos = QOS;
    pubmsg.retained = 0;
    deliveredtoken = 0;

    if ((rc = MQTTAsync_sendMessage(_client, topic.c_str(), &pubmsg, &_opts)) !=
        MQTTASYNC_SUCCESS) {
        printf("Failed to start sendMessage, return code %d\n", rc);
    }
}

void MqttBridge::onConnect(void* context, MQTTAsync_successData* response) {
    MqttBridge* me = (MqttBridge*)context;
    me->_opts = MQTTAsync_responseOptions_initializer;
    MQTTAsync_message pubmsg = MQTTAsync_message_initializer;
    int rc;

    printf("Successful connection\n");

    me->_opts.onSuccess = onSend;
    me->_opts.context = me;

    pubmsg.payload = (void*)PAYLOAD;
    pubmsg.payloadlen = (int)strlen(PAYLOAD);
    pubmsg.qos = QOS;
    pubmsg.retained = 0;
    deliveredtoken = 0;

    if ((rc = MQTTAsync_sendMessage(me->_client, TOPIC, &pubmsg, &me->_opts)) !=
        MQTTASYNC_SUCCESS) {
        printf("Failed to start sendMessage, return code %d\n", rc);
        exit(EXIT_FAILURE);
    }
}
