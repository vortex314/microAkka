#include "/home/lieven/workspace/Common/BipBuffer.cpp"
#include "/home/lieven/workspace/Common/Bytes.cpp"
#include "/home/lieven/workspace/Common/Cbor.cpp"
#include "/home/lieven/workspace/Common/CborQueue.cpp"
#include "/home/lieven/workspace/Common/Log.cpp"
#include "/home/lieven/workspace/Common/Semaphore.cpp"
#include "/home/lieven/workspace/Common/Str.cpp"
#include "/home/lieven/workspace/microAkka/src/Akka.cpp"
#include "/home/lieven/workspace/microAkka/src/Sys.cpp"
#include "/home/lieven/workspace/microAkka/src/Uid.cpp"
#include <Arduino.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <WiFi.h>

#define MAX_MESSAGES 10000
Log logger(256);
ActorSystem actorSystem(Sys::hostname());
Mailbox defaultMailbox("default", 5000, 256);
Mailbox coRoutineMailbox("coRoutine", 5000, 256);
Mailbox remoteMailbox("$remote", 5000, 256);
MessageDispatcher defaultDispatcher;
ActorRef echo;
ActorRef mqttBridge;
class MqttBridge;

WiFiClient espClient;
PubSubClient client(espClient);

const static MsgClass PONG("PONG");
const static MsgClass PING("PING");
const static MsgClass MQTT_PUBLISH_RCVD("MQTT_PUBLISH_RCVD");
const static MsgClass MQTT_IS_CONNECTED("MQTT_CONNECTED");

#define QOS 0

class Echo : public AbstractActor {
    Str _str;
    uint32_t _counter;
    ActorRef remote;
    String topic;

  public:
    Echo(va_list args) : str(80) {}
    ~Echo() {}

    void preStart() {
        INFO("started");
        timers().startPeriodicTimer("PERIODIC_TIMER_1", TimerExpired, 5000);
        remote = actorSystem.actorFor("DEVICE/ACTOR");
    }
    Receive& createReceive() {
        return receiveBuilder()
            .match(PING,
                   [this](Envelope& msg) {
                       msg.scanf("uS", &_counter, &_str);
                       sender().tell(self(), PONG, msg.id, "us", _counter,
                                     _str.c_str());
                   })
            .match(TimerExpired,
                   [this](Envelope& msg) {
                       INFO(" message received %s:%s:%s in %s",
                            msg.sender.path(), msg.receiver.path(),
                            msg.msgClass.label(), context().self().path());
                       remote.tell(self(), "HELLO", "s", "WORLD");
                   })
            .build();
    }
};

class MqttBridge : public AbstractActor {
    Str str;
    bool _connected;
    StaticJsonBuffer<2000> _jsonBuffer;
    String _clientId;
    String _address;
    static MqttBridge* me;

  public:
    MqttBridge(va_list args) : str(80) {
        const char* host = va_arg(args, const char*);
        int port = va_arg(args, int);
        me=this;
        INFO(" host:port %s:%d  ", host, port);
    }
    ~MqttBridge() {}

    void preStart() {
        timers().startPeriodicTimer("PERIODIC_TIMER_1", TimerExpired, 5000);
    }
    Receive& createReceive() {
        return receiveBuilder()
            .match(AnyClass,
                   [this](Envelope& msg) {
                       if (!(msg.receiver == self())) {
                           INFO(" message received %s:%s:%s [%d] in %s",
                                msg.sender.path(), msg.receiver.path(),
                                msg.msgClass.label(), msg.message.length(),
                                context().self().path());
                           _jsonBuffer.clear();
                           JsonArray& array = _jsonBuffer.createArray();
                           array.add(msg.receiver.path());
                           array.add(msg.sender.path());
                           array.add(msg.msgClass.label());
                           array.add(msg.id);
                           payloadToJsonArray(array, msg.message);

                           String topic = "dst/";
                           topic += msg.receiver.path();

                           String message;
                           array.printTo(message);

                           mqttPublish(topic.c_str(), message.c_str());
                       }
                   })
            .match(MQTT_PUBLISH_RCVD,
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
            .match(TimerExpired,
                   [this](Envelope& msg) {
                       INFO(" Heap Size %d ",ESP.getFreeHeap());
                       static String topic;
                       topic = "dst/";
                       topic += Sys::hostname();
                       topic += "/#";
                       mqttSubscribe(topic.c_str());
                   })
            .build();
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
                array.add((char*)str.c_str());
                // const char* is not copied into buffer
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

    bool handleMqttMessage(const char* message) {
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
            } else if (array.is<int>(i)) {
                envelope.message.add(array.get<int>(i));
            } else if (array.is<double>(i)) {
                envelope.message.add(array.get<double>(i));
            }
        }
        envelope.header(rcv, snd, cls, id);
        rcv.tell(snd, envelope);
        return true;
    }
    void mqttPublish(const char* topic, const char* message) {
        INFO(" MQTT TXD : %s = %s", topic, message);
        client.publish(topic, message);
    }

    void mqttSubscribe(const char* topic) {
        INFO("Subscribing to topic '%s' ", topic);
        return;
        if ( client.subscribe(topic)==false) {
            WARN(" MQTT subscribe failed ");
        }
    }

    static void callback(char* topic, byte* payload, unsigned int length) {
        Serial.print("Message arrived [");
        Serial.print(topic);
        Serial.print("] ");
        for (int i = 0; i < length; i++) {
            Serial.print((char)payload[i]);
        }
        Serial.println();
 //       me->onMessageArrived()
    }
};


MqttBridge* MqttBridge::me=0;

void reconnect() {
    // Loop until we're reconnected
    while (!client.connected()) {
        Serial.print("Attempting MQTT connection...");
        // Attempt to connect
        if (client.connect("ESP8266Client" + ::millis())) {
            Serial.println("connected");
            // Once connected, publish an announcement...
            client.publish("outTopic", "hello world - limero");
            // ... and resubscribe
            client.subscribe("dst/esp32/echo");
        } else {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" try again in 5 seconds");
            // Wait 5 seconds before retrying
            delay(1000);
        }
    }
}

void setup() {
    // initialize digital pin 13 as an output.
    pinMode(2, OUTPUT);
    Serial.begin(115200);
    WiFi.begin("Merckx3", "LievenMarletteEwoutRonald");
    client.setServer("test.mosquitto.org", 1883);
    client.setCallback(MqttBridge::callback);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Sys::hostname("esp32");
    echo = actorSystem.actorOf<Echo>("echo");
    mqttBridge = actorSystem.actorOf<MqttBridge>("mqttBridge",
                                                 "test.mosquitto.org", 1883);

    defaultDispatcher.attach(defaultMailbox);
    defaultDispatcher.attach(remoteMailbox);
    defaultDispatcher.attach(*ActorContext::context(echo));
    defaultDispatcher.unhandled(ActorContext::context(mqttBridge));
}

// the loop function runs over and over again forever
void loop() {

    digitalWrite(2, HIGH); // turn the LED on (HIGH is the voltage level)
    delay(100);            // wait for a second
    digitalWrite(2, LOW);  // turn the LED off by making the voltage LOW
    delay(100);            // wait for a second
    if (!client.connected()) {
        reconnect();
    }
    defaultDispatcher.execute();
}
