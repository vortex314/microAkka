#include <Bridge.h>
#include <sys/types.h>
#include <unistd.h>
// volatile MQTTAsync_token deliveredtoken;

/*
 * use cases :
 * 1/ send&receive simple properties
 * 	RXD ==> src/drive/motor/rpmMeasured = 122
 * 				-> dst=0 src=remoteRef cls=rpmMeasured value=122
 * 	TXD ==> src=localRef cls=rpmMeasured value=122
 *
 * 2/ send&receive complex properties
 * 	RXD ==> src/anchor1/dwm1000/location = {"x":1234,"y":6777 ,"id":8769}
 * 			-> dst=0 src=localRef cls=location x=1234, y=6777
 * id=8769
 * 	TXD ==> src=localRef cls=location x=12234 y=6777 id=8769
 *
 * 3/ send&receive simple commands
 * 	RXD ==> dst/motor/drive/targetRpm = 134
 * 				-> dst=localRef cls=targetRpm value=134
 * 	TXD ==> dst=remoteRef , src=localRef ,cls=targetRpm value=134
 *
 * 4/ send&receive complex commands
 * 		==> dst/motor/drive/set = {"$SRC":"dst/brain/pathFinder",
 * "targetRpm":134,"$ID":1299 ,"maxAmp":1.5 }
 * 	RXD ==> dst/motor/lps/location  = {"x":1222,"y":66776 }
 * 			-> dst=localRef src=0 cls=location x=1222 y=66776
 * 	TXD ==> dst=remoteRef , src=localRef , cls=set ,targetRpm=134
 * 	RXD ==> dst/esp321/system/reset = {"pass":"Anakwaboe" }
 * 			-> dst=localRef cls=reset
 *
 *  5/ Properties reply
 * 	TXD ==> src=localRef cls=PropertiesReply k1=v1,k2=v2 --> src/localRef/k1
 * = v1 .....
 *
 *  6/ To Bridge => local Subscribe
 * 				TXD ==> dst=bridge src=localRef cls=Subscribe
 *
 * 	1 to 1 mapping of MQTT & Xdr
 * Bridge subscribe at demand for local events
 * Bridge will subscribe to remote events by eventbus subscribers
 * Bridge can be asked to publish explicitly "publish"
 *
 */

MsgClass Bridge::Publish("publish");

Bridge::Bridge(ActorRef& mqtt) : _mqtt(mqtt) {
    _rxd = 0;
    _txd = 0;
    _mqttConnected = false;
}

Bridge::~Bridge() {}

void Bridge::preStart() {
    timers().startPeriodicTimer("publish", Msg("pollTimer"), 1000);
    _currentActorRef = context().system().actorRefs().begin();
    timers().startPeriodicTimer("pubTimer", Msg("pubTimer"), 5000);
    eb.subscribe(self(), MessageClassifier(_mqtt, Mqtt::Connected));
    eb.subscribe(self(), MessageClassifier(_mqtt, Mqtt::Disconnected));
    eb.subscribe(self(), MessageClassifier(_mqtt, Mqtt::PublishRcvd));
    Uid("value");
}

ActorRef* Bridge::nextRef() {
    if (context().system().actorRefs().size() == 0)
        return 0;
    if (_currentActorRef == context().system().actorRefs().end()) {
        _currentActorRef = context().system().actorRefs().begin();
    }
    ActorRef* pa = _currentActorRef->second;
    ++_currentActorRef;
    return pa;
}

Receive& Bridge::createReceive() {
    return receiveBuilder()

        .match(MsgClass::AnyClass,
               [this](Msg& msg) {
                   if (!(msg.dst() == self().id())) {
                       std::string message;
                       std::string topic;
                       if (msg.dst() == 0) { // cmd req/reply
                           msgToJsonEvents(msg);
                       } else { // event
                           msgToJsonCmd(topic, message, msg);
                           _mqtt.tell(msgBuilder(Mqtt::Publish)("topic", topic)(
                                          "message", message),
                                      self());
                           _txd++;
                       }
                   }
               })

        .match(Mqtt::Connected,
               [this](Msg& env) {
                   INFO(" MQTT CONNECTED");
                   _mqttConnected = true;
                   subscribeEventBus();
               })

        .match(Mqtt::Disconnected,
               [this](Msg& env) {
                   INFO(" MQTT DISCONNECTED");
                   _mqttConnected = false;
               })

        .match(Mqtt::PublishRcvd,
               [this](Msg& msg) {
                   std::string topic;
                   std::string message;

                   if (msg.get("topic", topic) && msg.get("message", message)) {
                       Msg& m = msgBuilder((uid_type)0);
                       m.src(ActorRef::NoSender().id());

                       if (topicToMsg(m, topic) && messageToMsg(m, message)) {
                           ActorRef *dst, *src;
                           if (m.dst() != 0 &&
                               (dst = ActorRef::lookup(m.dst())) != 0) {
                               dst->tell(m);
                               _rxd++;
                           } else if (m.src() != 0 &&
                                      (src = ActorRef::lookup(m.src())) != 0) {
                               eb.publish(m);
                               _rxd++;
                           } else {
                               WARN(" src / dst invalid %u / % u", m.src(),
                                    m.dst());
                           }
                       } else {
                           WARN(" couldn't handle topic/message %s=%s",
                                topic.c_str(), message.c_str());
                       }
                   }
               })

        .match(MsgClass("pubTimer"),
               [this](Msg& msg) {
                   std::string topic = "src/";
                   topic += context().system().label();
                   topic += "/system/alive";
                   if (_mqttConnected) {
                       _mqtt.tell(msgBuilder(Mqtt::Publish)("topic", topic)(
                                      "message", "true"),
                                  self());
                   }
                   self().tell(msgBuilder(Publish)("simple", 1123), self());

                   Xdr xdr(10);
                   xdr("a", 1)("b", true)("c", "a string")("e", 3.14);
                   self().tell(msgBuilder(Publish)("complex", xdr), self());

               })

        .match(MsgClass::PropertiesReply(),
               [this](Msg& msg) {
                   if (_mqttConnected == true) {
                       msgToJsonEvents(msg);
                   }
               })

        .match(MsgClass("pollTimer"),
               [this](Msg& msg) {
                   if (_mqttConnected == true) {
                       ActorRef* ref = nextRef();
                       if (ref)
                           ref->tell(msgBuilder(MsgClass::Properties())
                                         .src(self().id())
                                         .dst(ref->id()));
                   }
               })

        .match(Publish,
               [this](Msg& msg) {
                   if (_mqttConnected == true) {
                       msgToJsonEvents(msg);
                   }
               })

        .match(MsgClass::Properties(),
               [this](Msg& msg) {
                   sender().tell(replyBuilder(msg)("txd", _txd)("rxd", _rxd),
                                 self());
               })

        .build();
}
/*
 * cls==Publish or cls==PropertiesReply
 *
 * msg => src,cls,<tag:value> ==> src/device/<src>/<tag> = value
 * msg => src,cls,<tag:value>,<tag1:value1> ==> src/device/<src>/<cls> = { "tag"
 * : value,"tag1":"value1" }
 *
 */

bool xdrToJson(JsonVariant variant, Xdr& xdr) {
    std::string js;
    serializeJson(variant, js);
    while (xdr.hasData()) {
        Tag tag(xdr.peek());
        Uid tag_uid(tag.uid);
        if (tag.uid == UD_DST || tag.uid == UD_SRC || tag.uid == UD_CLS ||
            tag.uid == UD_ID) {
            xdr.skip();
        } else {
            if (tag.type == Xdr::BYTES) {
                std::string bytes;
                xdr.getNext(tag.uid, bytes);
                if (variant.isNull())
                    variant.set(bytes);
                else if (variant.is<JsonObject>())
                    variant[tag_uid.label()] = bytes;
            } else if (tag.type == Xdr::UINT) {
                uint64_t ui64;
                xdr.getNext(tag.uid, ui64);
                if (variant.isNull())
                    variant.set(ui64);
                else if (variant.is<JsonObject>())
                    variant[tag_uid.label()] = ui64;
            } else if (tag.type == Xdr::INT) {
                int64_t i64;
                xdr.getNext(tag.uid, i64);
                if (variant.isNull())
                    variant.set(i64);
                else if (variant.is<JsonObject>())
                    variant[tag_uid.label()] = i64;
            } else if (tag.type == Xdr::FLOAT) {
                double d;
                xdr.getNext(tag.uid, d);
                if (variant.isNull())
                    variant.set(d);
                else if (variant.is<JsonObject>())
                    variant[tag_uid.label()] = d;
            } else if (tag.type == Xdr::BOOL) {
                bool b;
                xdr.get(tag.uid, b);
                if (variant.isNull())
                    variant.set(b);
                else if (variant.is<JsonObject>())
                    variant[tag_uid.label()] = b;
            } else if (tag.type == Xdr::OBJECT) {
                Xdr xdrChild(20);
                xdr.getNext(tag.uid, xdrChild);
                Tag childTag(xdrChild.peek());
                JsonObject object = variant.to<JsonObject>();
                //                  variant.createNestedObject(tag_uid.label());
                xdrChild.rewind();
                xdrToJson(object, xdrChild);
            } else {
                WARN(" unknown xdr Type ");
                xdr.skip();
            }
        }
    }
    return true;
}

bool Bridge::msgToJson(Msg& msg, std::string& topic, std::string& message) {
    if (msg.dst()) {
        topic = "dst/";
        topic += Uid::label(msg.dst());
        topic += "/";
        topic += Uid::label(msg.cls());
        return msgToJsonCmd(topic, message, msg);
    } else if (msg.src()) {
        return msgToJsonEvents(msg);
    } else
        return false;
}
// TODO check for 1 value or multiple => JsVariant or JsObject
bool Bridge::msgToJsonEvents(Msg& msg) {
    std::string topic;
    std::string message;
    Tag tag(0);
    if (msg.cls() == MsgClass::PropertiesReply().id()) {
        msg.rewind();
        Msg& pub = msgBuilder(Mqtt::Publish);
        while (msg.hasData()) {
            Tag tag(msg.peek());
            Uid tag_uid(tag.uid);
            if (tag.uid == UD_DST || tag.uid == UD_SRC || tag.uid == UD_CLS ||
                tag.uid == UD_ID) {
                msg.skip();
            } else {
                _jsonDoc.clear();
                message.clear();
                JsonVariant variant = _jsonDoc.to<JsonVariant>();
                topic = "src/";
                topic += sender().path();
                topic += "/";
                topic += tag_uid.label();
                xdrToJson(variant, msg);
                serializeJson(_jsonDoc, message);
                pub("topic", topic)("message", message);
            }
        }
        _mqtt.tell(pub, self());
        return true;
    } else {
        _jsonDoc.clear();
        JsonVariant variant = _jsonDoc.to<JsonVariant>();

        msg.rewind();
        Msg& pub = msgBuilder(Mqtt::Publish);
        while (msg.hasData()) {
            Tag tag(msg.peek());
            Uid tag_uid(tag.uid);
            topic = "src/";
            topic += sender().path();
            topic += "/";
            topic += tag_uid.label();
            if (tag.uid == UD_DST || tag.uid == UD_SRC || tag.uid == UD_CLS ||
                tag.uid == UD_ID) {
                msg.skip();
            } else {
                xdrToJson(variant, msg);
                serializeJson(_jsonDoc, message);
                pub("topic", topic)("message", message);
                _mqtt.tell(pub, self());
            }
        }

        return true;
    }
}
/*
 * count fields excluding dst,src,cls
 * tag references the last one found
 */
uint32_t Bridge::fieldCount(Msg& msg, Tag& tag) {
    Tag cursor(0);
    uint32_t fields = 0;
    msg.rewind();
    while (msg.hasData()) {
        cursor.ui32 = msg.peek();
        if (tag.uid == UD_DST || tag.uid == UD_SRC || tag.uid == UD_CLS) {
        } else {
            tag = cursor;
            fields++;
        }
        msg.skip();
    }
    return fields;
}

bool Bridge::msgToJsonCmd(std::string& topic, std::string& message, Msg& msg) {
    Tag tag(0);
    std::string str;

    msg.rewind();
    _jsonDoc.clear();
    JsonObject jsonObject = _jsonDoc.to<JsonObject>();
    topic = "dst/";
    topic += Uid::label(msg.dst());
    topic += "/";
    topic += Uid::label(msg.cls());

    if (fieldCount(msg, tag) == 1) {
        switch (tag.type) {
        case Xdr::BYTES: {
            msg.getNext(tag.uid, str);
            jsonObject[Uid::label(tag.uid)] =
                (char*)str.c_str(); // cast to char * to enforce copy into
            // ArduinoJson buffer
            break;
        }
        case Xdr::UINT: {
            uint64_t u64;
            msg.getNext(tag.uid, u64);
            if (tag.uid == UD_DST || tag.uid == UD_SRC || tag.uid == UD_CLS) {
                if (tag.uid == UD_SRC)
                    jsonObject[Uid::label(tag.uid)] = Uid::label(u64);
            } else {
                jsonObject[Uid::label(tag.uid)] = u64;
            }
            break;
        }
        case Xdr::INT: {
            int64_t i64;
            msg.getNext(tag.uid, i64);
            jsonObject[Uid::label(tag.uid)] = i64;
            break;
        }
        case Xdr::FLOAT: {
            double d;
            msg.getNext(tag.uid, d);
            jsonObject[Uid::label(tag.uid)] = d;
            break;
        }
        default: { msg.skip(); }
        }

        // serialize the object and send the result to Serial
        serializeJson(jsonObject, message);
        return true;
    } else {

        while (msg.hasData()) {
            tag.ui32 = msg.peek();
            //		Uid tagUid=Uid(tag.uid);
            switch (tag.type) {
            case Xdr::BYTES: {
                msg.getNext(tag.uid, str);
                jsonObject[Uid::label(tag.uid)] =
                    (char*)str.c_str(); // cast to char * to enforce copy
                break;
            }
            case Xdr::UINT: {
                uint64_t u64;
                msg.getNext(tag.uid, u64);
                if (tag.uid == UD_DST || tag.uid == UD_SRC ||
                    tag.uid == UD_CLS) {
                    if (tag.uid == UD_SRC)
                        jsonObject[Uid::label(tag.uid)] = Uid::label(u64);
                } else {
                    jsonObject[Uid::label(tag.uid)] = u64;
                }
                break;
            }
            case Xdr::INT: {
                int64_t i64;
                msg.getNext(tag.uid, i64);
                jsonObject[Uid::label(tag.uid)] = i64;
                break;
            }
            case Xdr::FLOAT: {
                double d;
                msg.getNext(tag.uid, d);
                jsonObject[Uid::label(tag.uid)] = d;
                break;
            }
            default: { msg.skip(); }
            }
        };
        serializeJson(jsonObject, message);
        return true;
    }
}

bool jsonToXdr(Xdr& xdr, const char* name, JsonVariant variant) {
    if (variant.is<JsonObject>()) {
        JsonObject jsonObject = variant.as<JsonObject>();
        for (JsonPair kv : jsonObject) {
            if (strcmp(kv.key().c_str(), AKKA_SRC) == 0) {
            } else {
                if (kv.value().is<char*>()) {
                    xdr(kv.key().c_str(), kv.value().as<char*>());
                } else if (kv.value().is<unsigned long>()) {
                    xdr(kv.key().c_str(), kv.value().as<uint64_t>());
                } else if (kv.value().is<long>()) {
                    xdr(kv.key().c_str(), kv.value().as<int64_t>());
                } else if (kv.value().is<double>()) {
                    xdr(kv.key().c_str(), kv.value().as<double>());
                } else if (kv.value().is<bool>()) {
                    xdr(kv.key().c_str(), kv.value().as<bool>());
                } else if (kv.value().is<JsonObject>()) {
                    Xdr xdrChild(10);
                    jsonToXdr(xdrChild, "NONAME", kv.value().as<JsonObject>());
                    xdr(kv.key().c_str(), xdrChild);
                } else {
                    WARN(" unhandled JSON type ");
                    return false;
                }
            }
        }
        return true;
    } else if (variant.is<char*>()) {
        xdr(H(name), variant.as<char*>());
    } else if (variant.is<unsigned long>()) {
        xdr(H(name), variant.as<uint64_t>());
    } else if (variant.is<long>()) {
        xdr(H(name), variant.as<int64_t>());
    } else if (variant.is<double>()) {
        xdr(H(name), variant.as<double>());
    } else if (variant.is<bool>()) {
        xdr(H(name), variant.as<bool>());
    } else {
        WARN(" unhandled JSON type ");
        return false;
    }
	return true;
}

bool Bridge::messageToMsg(Msg& msg, std::string& message) {
    _jsonDoc.clear();
    auto rc = deserializeJson(_jsonDoc, message.data());
    if (!(rc == DeserializationError::Ok)) { // just read this as a string
        msg(H("value"), message.c_str());
        return true;
    }

    JsonObject jsonObject = _jsonDoc.as<JsonObject>();
    if (!jsonObject.isNull()) {
        for (JsonPair kv : jsonObject) {
            if (strcmp(kv.key().c_str(), AKKA_SRC) == 0) {
                RemoteActorRef* lr =
                    new RemoteActorRef(Uid(kv.value().as<char*>()), self());
                msg.src(lr->id());
            } else {
                if ( !jsonToXdr(msg, kv.key().c_str(), jsonObject)) return false;
            }
        }
        return true;
    } else {
        JsonVariant variant = _jsonDoc.as<JsonVariant>();
        return jsonToXdr(msg, "value", variant);
    }
}

bool Bridge::topicToMsg(Msg& msg, std::string& topic) {
    uint32_t offsets[3] = {0, 0, 0};
    uint32_t prevOffset = 0;
    for (uint32_t i = 0; i < 3; i++) {
        uint64_t offset = topic.find(
            '/', prevOffset); // uint64_t to support 64 bit architecture ;-)
        if (offset == std::string::npos)
            break;
        offsets[i] = offset;
        prevOffset = offset + 1;
    }
    if (offsets[0] == 0 || offsets[2] == 0) {
        WARN(" invalid topic : %s", topic.c_str());
        return false;
    }
    uid_type uid =
        Uid(topic.substr(offsets[0] + 1, offsets[2] - offsets[0] - 1).c_str())
            .id();
    uid_type uidCls = Uid(topic.substr(offsets[2] + 1).c_str()).id();
    if (topic.rfind("dst/", 0) == 0) {
        ActorRef* dst = ActorRef::lookup(uid);
        if (dst == 0) {
            WARN(" local Actor : %s not found ", Uid::label(uid));
            return false;
        }
        msg.dst(uid);
    } else if (topic.rfind("src/", 0) == 0) {
        ActorRef* src = ActorRef::lookup(uid);
        if (src == 0) { // TODO -> bridge mailbox, no cell, tell & forward diff
            src = new RemoteActorRef(Uid(uid), self());
        }
        msg.src(uid);
    }
    msg.cls(uidCls);
    return true;
}

void Bridge::subscribeEventBus() {
    for (auto subscriber : eb.subscribers()) {
        auto cl = subscriber->_classifier;
        DEBUG(" looking up : %s", Uid::label(cl.src()));
        ActorRef* ref = ActorRef::lookup(cl.src());
        if (ref != 0 && ref->isLocal() == false) {
            std::string topic = "src/";
            topic += Uid::label(cl.src());
            topic += "/";
            topic += Uid::label(cl.cls());
            _mqtt.tell(msgBuilder(Mqtt::Subscribe)("topic", topic.c_str()),
                       self());
        }
    }
}
