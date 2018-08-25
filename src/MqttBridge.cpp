
#include <MqttBridge.h>

volatile MQTTAsync_token deliveredtoken;

MqttBridge::MqttBridge(){};
MqttBridge::~MqttBridge() {}

void MqttBridge::preStart() {
    context().mailbox(remoteMailbox);
    _conn_opts = MQTTAsync_connectOptions_initializer;
    int rc;

    MQTTAsync_create(&_client, ADDRESS, CLIENTID, MQTTCLIENT_PERSISTENCE_NONE,
                     NULL);

    MQTTAsync_setCallbacks(_client, this, onConnectionLost, onMessageArrived,
                           onDeliveryComplete);

    _conn_opts.keepAliveInterval = 20;
    _conn_opts.cleansession = 1;
    _conn_opts.onSuccess = onConnect;
    _conn_opts.onFailure = onConnectFailure;
    _conn_opts.context = this;
    if ((rc = MQTTAsync_connect(_client, &_conn_opts)) != MQTTASYNC_SUCCESS) {
        INFO("Failed to start connect, return code %d", rc);
    }
}


Receive& MqttBridge::createReceive() {
    return receiveBuilder()
        .match(AnyClass,
               [this](Envelope& msg) {
                   INFO(" message received %s:%s:%s in %s", msg.sender.path(),
                        msg.receiver.path(), msg.msgClass.label(),
                        context().self().path());
                   _jsonBuffer.clear();
                   JsonArray& array = _jsonBuffer.createArray();
                   array.add(msg.receiver.path());
                   array.add(msg.sender.path());
                   array.add(msg.msgClass.label());
                   array.add(msg.id);
                   std::string topic = "dst/";
                   topic += msg.receiver.path();
                   std::string message;
                   array.printTo(message);
                   mqttPublish(topic.c_str(), message.c_str());
               })
        .match(MQTT_PUBLISH_RCVD,
               [this](Envelope& msg) {
                   Str topic(100);
                   Str message(1024);

                   if (msg.scanf("SS", topic, message) &&
                       handleMqttMessage(message.c_str())) {
                       INFO(" processed message %s", message.c_str());
                   } else {
                       WARN(" processing failed : %s ", message.c_str());
                   }
               })
        .build();
}


bool MqttBridge::handleMqttMessage(const char* message) {
    Envelope envelope(1024);
    _jsonBuffer.clear();
    JsonArray& array = _jsonBuffer.parse(message);
    if (array == JsonArray::invalid())
        return false;
    if (array.size() < 4)
        return false;
    if (!array.is<char*>(0) || !array.is<char*>(1) || !array.is<char*>(2))
        return false;

    ActorRef rcv(array.get<const char*>(0));
    ActorRef snd(array.get<const char*>(1));
    MsgClass cls(array.get<const char*>(2));
    uint16_t id = array.get<int>(3);
    for (uint32_t i = 4; i < array.size(); i++) {
        // TODO add to cbor buffer
        if (array.is<char*>(i)) {
            envelope.message.add(array.get<const char*>(i));
        } else if (array.is<double>(i)) {
            envelope.message.add(array.get<double>(i));
        }
    }
    envelope.header(rcv, snd, cls, id);
    rcv.tell(snd, envelope);
    return true;
}

void MqttBridge::onConnect(void* context, MQTTAsync_successData* response) {
    MqttBridge* me = (MqttBridge*)context;
    INFO("Successful connection");
    std::string topic = "dst/";
    topic += me->context().system().label();
    topic += "/#";
    me->mqttSubscribe(topic.c_str());
}

void MqttBridge::onConnectionLost(void* context, char* cause) {
    MqttBridge* me = (MqttBridge*)context;
    me->_connected = false;
    me->_conn_opts = MQTTAsync_connectOptions_initializer;
    int rc;

    WARN(" connection lost ! cause: %s", cause);

    INFO("Reconnecting");
    me->_conn_opts.keepAliveInterval = 20;
    me->_conn_opts.cleansession = 1;
    if ((rc = MQTTAsync_connect(me->_client, &me->_conn_opts)) !=
        MQTTASYNC_SUCCESS) {
        WARN("Failed to start connect, return code %d", rc);
    }
}

void MqttBridge::onConnectFailure(void* context,
                                  MQTTAsync_failureData* response) {
    //    MqttBridge* me = (MqttBridge*)context;
    WARN("Connect failed, rc %d", response ? response->code : 0);
}

void MqttBridge::onDisconnect(void* context, MQTTAsync_successData* response) {
    MqttBridge* me = (MqttBridge*)context;
    INFO("Successful disconnection %X", me);
}

void MqttBridge::onSend(void* context, MQTTAsync_successData* response) {
    //    MqttBridge* me = (MqttBridge*)context;
    INFO("Message with token value %d onSend", response->token);
}

void MqttBridge::onDeliveryComplete(void* context, MQTTAsync_token response) {
    //    MqttBridge* me = (MqttBridge*)context;
    INFO("Message with token value %d onDeliveryComplete", response);
}

void MqttBridge::onSubscribeFailure(void* context,
                                    MQTTAsync_failureData* response) {
    //    MqttBridge* me = (MqttBridge*)context;
    WARN("Subscribe failed, rc %d", response ? response->code : 0);
}

void MqttBridge::onSubscribe(void* context, MQTTAsync_successData* response) {
    //    MqttBridge* me = (MqttBridge*)context;
    INFO("Subscribe success");
}

int MqttBridge::onMessageArrived(void* context, char* topicName, int topicLen,
                                 MQTTAsync_message* message) {
                                     INFO(" message arrived");
    MqttBridge* me = (MqttBridge*)context;
    Str topic((uint8_t*)topicName, topicLen);
    Str msg((uint8_t*)message->payload, message->payloadlen);
    me->self().tell(me->self(), MQTT_PUBLISH_RCVD, "SS", &topicName, &msg);
    MQTTAsync_freeMessage(&message);
    MQTTAsync_free(topicName);
    return 1;
}

void MqttBridge::mqttPublish(const char* topic, const char* message) {
    INFO(" MQTT PUB %s:%s", topic, message);
    MQTTAsync_message pubmsg = MQTTAsync_message_initializer;
    int rc;
    _opts.onSuccess = onSend;
    _opts.context = this;

    pubmsg.payload = (void*)message;
    pubmsg.payloadlen = (int)strlen(message);
    pubmsg.qos = QOS;
    pubmsg.retained = 0;
    deliveredtoken = 0;

    if ((rc = MQTTAsync_sendMessage(_client, topic, &pubmsg, &_opts)) !=
        MQTTASYNC_SUCCESS) {
        INFO("Failed to start sendMessage, return code %d", rc);
    }
}

void MqttBridge::mqttSubscribe(const char* topic) {
    INFO("Subscribing to topic %s for client %s using QoS%d", topic, CLIENTID,
         QOS);
    _opts.onSuccess = onSubscribe;
    _opts.onFailure = onSubscribeFailure;
    _opts.context = this;

    deliveredtoken = 0;
    int rc;

    if ((rc = MQTTAsync_subscribe(_client, topic, QOS, &_opts)) !=
        MQTTASYNC_SUCCESS) {
        INFO("Failed to start subscribe, return code %d", rc);
    }
}
