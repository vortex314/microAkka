
#include <MqttBridge.h>
#include <sys/types.h>
#include <unistd.h>
// volatile MQTTAsync_token deliveredtoken;

MqttBridge::MqttBridge(va_list args) { _address = va_arg(args, const char*); };
MqttBridge::~MqttBridge() {}

MsgClass MqttBridge::MQTT_PUBLISH_RCVD() {
    static MsgClass MQTT_PUBLISH_RCVD("MQTT_PUBLISH_RCVD");
    return MQTT_PUBLISH_RCVD;
}

void MqttBridge::preStart() {
    //   context().mailbox(remoteMailbox);
    _conn_opts = MQTTAsync_connectOptions_initializer;

    _clientId = self().path();
    _clientId += "#";
    char str[10];
    int pid = getpid();
    snprintf(str, sizeof(str), "%d", pid);
    _clientId += str;

    MQTTAsync_create(&_client, _address.c_str(), _clientId.c_str(),
                     MQTTCLIENT_PERSISTENCE_NONE, NULL);

    MQTTAsync_setCallbacks(_client, this, onConnectionLost, onMessageArrived,
                           onDeliveryComplete);

    _conn_opts.keepAliveInterval = 600;
    _conn_opts.cleansession = 1;
    _conn_opts.onSuccess = onConnect;
    _conn_opts.onFailure = onConnectFailure;
    _conn_opts.context = this;
    mqttConnect();
}

void MqttBridge::mqttConnect() {
    int rc;
    INFO(" connecting to %s", _address.c_str());
    if ((rc = MQTTAsync_connect(_client, &_conn_opts)) != MQTTASYNC_SUCCESS) {
        INFO("Failed to start connect, return code %d", rc);
    }
}

void MqttBridge::mqttDisconnect() {
    int rc;
    MQTTAsync_disconnectOptions opt;
    opt.timeout = 10;
    if ((rc = MQTTAsync_disconnect(_client, &opt)) != MQTTASYNC_SUCCESS) {
        INFO("Failed to start connect, return code %d", rc);
    }
}

bool payloadToJsonArray(JsonArray& array, Cbor& payload) {
    Cbor::PackType pt;
    payload.offset(0);
    Erc rc;
    Str str(100);
    while (payload.hasData()) {
        rc = payload.peekToken(pt);
        if (rc != E_OK)
            break;
        switch (pt) {
        case Cbor::P_STRING: {
            payload.get(str);
            array.add(
                (char*)str.c_str()); // const char* is not copied into buffer
            break;
        }
        case Cbor::P_PINT: {
            uint64_t u64;
            payload.get(u64);
            array.add(u64);
            break;
        }
        case Cbor::P_DOUBLE: {
            double d;
            payload.get(d);
            array.add(d);
            break;
        }
        default: { payload.skipToken(); }
        }
    };
    return true;
}

Receive& MqttBridge::createReceive() {
    return receiveBuilder()
        .match(AnyClass,
               [this](Envelope& msg) {
                   if (!(*msg.receiver == self())) {
                       INFO(" message received %s:%s:%s [%d] in %s",
                            msg.sender->path(), msg.receiver->path(),
                            msg.msgClass.label(), msg.message.length(),
                            context().self().path());
                       _jsonBuffer.clear();
                       JsonArray& array = _jsonBuffer.createArray();
                       array.add(msg.receiver->path());
                       array.add(msg.sender->path());
                       array.add(msg.msgClass.label());
                       array.add(msg.id);
                       payloadToJsonArray(array, msg.message);

                       std::string topic = "dst/";
                       topic += msg.receiver->path();

                       std::string message;
                       array.printTo(message);

                       mqttPublish(topic.c_str(), message.c_str());
                   }
               })
        .match(MQTT_PUBLISH_RCVD(),
               [this](Envelope& msg) {
                   Str topic(100);
                   Str message(1024);

                   if (msg.scanf("SS", &topic, &message) &&
                       handleMqttMessage(message.c_str())) {
                       INFO(" processed message %s", message.c_str());
                   } else {
                       WARN(" processing failed : %s ", message.c_str());
                   }
               })
        .build();
}

bool MqttBridge::handleMqttMessage(const char* message) {
    //    Envelope envelope(1024);
    _jsonBuffer.clear();
    JsonArray& array = _jsonBuffer.parse(message);
    if (array == JsonArray::invalid())
        return false;
    if (array.size() < 4)
        return false;
    if (!array.is<char*>(0) || !array.is<char*>(1) || !array.is<char*>(2))
        return false;

    ActorRef* rcv = ActorRef::lookup(Uid::hash(array.get<const char*>(0)));
    if (rcv == 0) {
        rcv = &NoSender;
        WARN(" local Actor : %s not found ", array.get<const char*>(0));
    }
    ActorRef* snd = ActorRef::lookup(Uid::hash(array.get<const char*>(1)));
    if (snd == 0) {
        snd = new ActorRef(Uid::hash(array.get<const char*>(1)), 0);
    }
    MsgClass cls(array.get<const char*>(2));
    uint16_t id = array.get<int>(3);
    for (uint32_t i = 4; i < array.size(); i++) {
        // TODO add to cbor buffer
        if (array.is<char*>(i)) {
            txdEnvelope().message.add(array.get<const char*>(i));
        } else if (array.is<int>(i)) {
            txdEnvelope().message.add(array.get<int>(i));
        } else if (array.is<double>(i)) {
            txdEnvelope().message.add(array.get<double>(i));
        }
    }
    txdEnvelope().header(*rcv, *snd, cls, id);
    rcv->tell(*snd, txdEnvelope());
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
    me->_conn_opts.onSuccess = onConnect;
    me->_conn_opts.onFailure = onConnectFailure;
    me->_conn_opts.context = me;
    if ((rc = MQTTAsync_connect(me->_client, &me->_conn_opts)) !=
        MQTTASYNC_SUCCESS) {
        INFO("Failed to start connect, return code %d", rc);
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
    //   INFO("Message with token value %d onSend", response->token);
}

void MqttBridge::onDeliveryComplete(void* context, MQTTAsync_token response) {
    //    MqttBridge* me = (MqttBridge*)context;
    //    INFO("Message with token value %d onDeliveryComplete", response);
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
// send myself message as this is invoked by another thread

int MqttBridge::onMessageArrived(void* context, char* topicName, int topicLen,
                                 MQTTAsync_message* message) {
    MqttBridge* me = (MqttBridge*)context;
    Str topic((uint8_t*)topicName, topicLen);
    Str msg((uint8_t*)message->payload, message->payloadlen);
    INFO(" MQTT RXD : %s = %s ", topicName, message->payload);
    //   me->self().tell(me-self(),MQTT_PUBLISH_RCVD,"SS",&topic,&msg);
    Envelope envelope(1024);
    envelope.header(me->self(), me->self(), MQTT_PUBLISH_RCVD());
    envelope.message.addf("SS", &topic, &msg);
    me->self().tell(me->self(), envelope);
    MQTTAsync_freeMessage(&message);
    MQTTAsync_free(topicName);
    return 1;
}

void MqttBridge::mqttPublish(const char* topic, const char* message) {
    INFO(" MQTT TXD : %s = %s", topic, message);
    MQTTAsync_message pubmsg = MQTTAsync_message_initializer;
    int rc;
    _opts.onSuccess = onSend;
    _opts.context = this;

    pubmsg.payload = (void*)message;
    pubmsg.payloadlen = (int)strlen(message);
    pubmsg.qos = QOS;
    pubmsg.retained = 0;
    _deliveredtoken = 0;

    if ((rc = MQTTAsync_sendMessage(_client, topic, &pubmsg, &_opts)) !=
        MQTTASYNC_SUCCESS) {
        INFO("Failed to start sendMessage, return code %d", rc);
        mqttDisconnect();
        mqttConnect();
    }
}

void MqttBridge::mqttSubscribe(const char* topic) {
    INFO("Subscribing to topic %s for client %s using QoS%d", topic,
         _clientId.c_str(), QOS);
    _opts.onSuccess = onSubscribe;
    _opts.onFailure = onSubscribeFailure;
    _opts.context = this;

    _deliveredtoken = 0;
    int rc;

    if ((rc = MQTTAsync_subscribe(_client, topic, QOS, &_opts)) !=
        MQTTASYNC_SUCCESS) {
        INFO("Failed to start subscribe, return code %d", rc);
    }
}
