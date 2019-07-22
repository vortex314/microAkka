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
 * 				-> dst=0 src=remoteRef cls=location x=1234
 * y=6777
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
	Label("value");
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
				//			INFO(" to MQTT :
				//%s=%s",topic.c_str(),message.c_str());
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

bool Bridge::msgToJson(Msg& msg, std::string& topic, std::string& message) {
	if (msg.dst()) {
		topic = "dst/";
		topic += Label::label(msg.dst());
		topic += "/";
		topic += Label::label(msg.cls());
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
	INFO("%d : %d ",msg.cls(),MsgClass::PropertiesReply().id());
	if (msg.cls() == MsgClass::PropertiesReply().id()) {
		msg.rewind();
		Msg& pub = msgBuilder(Mqtt::Publish);
		while (msg.hasData()) {
			Tag tag(msg.peek());
			Label tag_uid(tag.uid);
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
				if (tag.type == Xdr::BYTES) {
					std::string bytes;
					msg.get(tag.uid, bytes);
					variant.set(bytes);
				} else if (tag.type == Xdr::UINT) {
					uint64_t ui64;
					msg.get(tag.uid, ui64);
					variant.set(ui64);
				} else if (tag.type == Xdr::INT) {
					int64_t i64;
					msg.get(tag.uid, i64);
					variant.set(i64);
				} else if (tag.type == Xdr::FLOAT) {
					double d;
					msg.get(tag.uid, d);
					variant.set(d);
				} else if (tag.type == Xdr::BOOL) {
					bool b;
					msg.get(tag.uid, b);
					variant.set(b);
				} else {
					msg.skip();
				}
				serializeJson(_jsonDoc, message);
				pub("topic", topic)("message", message);
			}
		}
		_mqtt.tell(pub, self());
		return true;
	} else if (fieldCount(msg, tag) == 1) {
		msg.rewind();
		Msg& pub = msgBuilder(Mqtt::Publish);
		topic = "src/";
		topic += sender().path();
		topic += "/";
		topic += Label::label(msg.cls());

		_jsonDoc.clear();
		JsonVariant variant = _jsonDoc.to<JsonVariant>();
		if (tag.type == Xdr::BYTES) {
			std::string bytes;
			msg.get(tag.uid, bytes);
			variant.set(bytes);
		} else if (tag.type == Xdr::UINT) {
			uint64_t ui64;
			msg.get(tag.uid, ui64);
			variant.set(ui64);
		} else if (tag.type == Xdr::INT) {
			int64_t i64;
			msg.get(tag.uid, i64);
			variant.set(i64);
		} else if (tag.type == Xdr::FLOAT) {
			double d;
			msg.get(tag.uid, d);
			variant.set(d);
		} else if (tag.type == Xdr::BOOL) {
			bool b;
			msg.get(tag.uid, b);
			variant.set(b);
		} else {
			msg.skip();
		}
		serializeJson(_jsonDoc, message);
		pub("topic", topic)("message", message);

		_mqtt.tell(pub, self());
		return true;
	} else {
		_jsonDoc.clear();
		JsonObject jsonObject = _jsonDoc.to<JsonObject>();
		topic = "src/";
		topic += sender().path();
		topic += "/";
		topic += Label::label(msg.cls());
		msg.rewind();

		Msg& pub = msgBuilder(Mqtt::Publish);
		while (msg.hasData()) {
			Tag tag(msg.peek());
			Label tag_uid(tag.uid);
			if (tag.uid == UD_DST || tag.uid == UD_SRC || tag.uid == UD_CLS ||
			        tag.uid == UD_ID) {
				msg.skip();
			} else {

				if (tag.type == Xdr::BYTES) {
					std::string bytes;
					msg.get(tag.uid, bytes);
					jsonObject[Label::label(tag.uid)] = bytes;
				} else if (tag.type == Xdr::UINT) {
					uint64_t ui64;
					msg.get(tag.uid, ui64);
					jsonObject[Label::label(tag.uid)] = ui64;
				} else if (tag.type == Xdr::INT) {
					int64_t i64;
					msg.get(tag.uid, i64);
					jsonObject[Label::label(tag.uid)] = i64;
				} else if (tag.type == Xdr::FLOAT) {
					double d;
					msg.get(tag.uid, d);
					jsonObject[Label::label(tag.uid)] = d;
				} else if (tag.type == Xdr::BOOL) {
					bool b;
					msg.get(tag.uid, b);
					jsonObject[Label::label(tag.uid)] = b;
				} else {
					msg.skip();
				}
			}
		}
		serializeJson(_jsonDoc, message);
		pub("topic", topic)("message", message);
		_mqtt.tell(pub, self());
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
	topic += Label::label(msg.dst());
	topic += "/";
	topic += Label::label(msg.cls());

	if (fieldCount(msg, tag) == 1) {
		switch (tag.type) {
			case Xdr::BYTES: {
					msg.getNext(tag.uid, str);
					jsonObject[Label::label(tag.uid)] =
					    (char*)str.c_str(); // cast to char * to enforce copy into
					// ArduinoJson buffer
					break;
				}
			case Xdr::UINT: {
					uint64_t u64;
					msg.getNext(tag.uid, u64);
					if (tag.uid == UD_DST || tag.uid == UD_SRC || tag.uid == UD_CLS) {
						if (tag.uid == UD_SRC)
							jsonObject[Label::label(tag.uid)] = Label::label(u64);
					} else {
						jsonObject[Label::label(tag.uid)] = u64;
					}
					break;
				}
			case Xdr::INT: {
					int64_t i64;
					msg.getNext(tag.uid, i64);
					jsonObject[Label::label(tag.uid)] = i64;
					break;
				}
			case Xdr::FLOAT: {
					double d;
					msg.getNext(tag.uid, d);
					jsonObject[Label::label(tag.uid)] = d;
					break;
				}
			default:
				{ msg.skip(); }
		}

		// serialize the object and send the result to Serial
		serializeJson(jsonObject, message);
		return true;
	} else {

		while (msg.hasData()) {
			tag.ui32 = msg.peek();
			//		Label tagLabel=Label(tag.uid);
			switch (tag.type) {
				case Xdr::BYTES: {
						msg.getNext(tag.uid, str);
						jsonObject[Label::label(tag.uid)] =
						    (char*)str.c_str(); // cast to char * to enforce copy
						break;
					}
				case Xdr::UINT: {
						uint64_t u64;
						msg.getNext(tag.uid, u64);
						if (tag.uid == UD_DST || tag.uid == UD_SRC ||
						        tag.uid == UD_CLS) {
							if (tag.uid == UD_SRC)
								jsonObject[Label::label(tag.uid)] = Label::label(u64);
						} else {
							jsonObject[Label::label(tag.uid)] = u64;
						}
						break;
					}
				case Xdr::INT: {
						int64_t i64;
						msg.getNext(tag.uid, i64);
						jsonObject[Label::label(tag.uid)] = i64;
						break;
					}
				case Xdr::FLOAT: {
						double d;
						msg.getNext(tag.uid, d);
						jsonObject[Label::label(tag.uid)] = d;
						break;
					}
				default:
					{ msg.skip(); }
			}
		};
		serializeJson(jsonObject, message);
		return true;
	}
}
/*
 *  dst/actorSystem/actor/message_type
 *
 *
 *
 *
 */
/*
bool Bridge::jsonCommandToMessage(Msg& msg, std::string& topic, std::string&
message) {
        _jsonDoc.clear();
        auto error = deserializeJson(_jsonDoc, message.data());
        JsonObject jsonObject = _jsonDoc.as<JsonObject>();

        if (error || jsonObject.isNull()) {
                WARN(" Invalid Json : %s ", message.c_str());
                return false;
        }

        if (!jsonObject.containsKey(AKKA_SRC)) {
                WARN(" missing source in json message : %s ",
message.c_str());
                return false;
        }
        uint32_t offsets[3] = { 0, 0, 0 };
        uint32_t prevOffset = 0;
        for (uint32_t i = 0; i < 3; i++) {
                uint64_t offset = topic.find('/', prevOffset); // uint64_t
to
support 64 bit architecture ;-)
                if (offset == std::string::npos) break;
                offsets[i] = offset;
                prevOffset = offset + 1;
        }
        if (offsets[0] == 0 || offsets[2] == 0) {
                WARN(" invalid topic : %s", topic.c_str());
                return false;
        }

//	INFO(" topic : %s %d,%d,%d", topic.c_str(), offsets[0], offsets[1],
offsets[2]);
//	INFO(" local actor : %s ", topic.substr(offsets[0] + 1, offsets[2]
//	                                        - offsets[0] - 1).c_str());
//	INFO(" local msg class : %s ", topic.substr(offsets[2] + 1).c_str());

        uid_type uidDst = Label(topic.substr(offsets[0] + 1, offsets[2] -
offsets[0]
                                             - 1).c_str()).id();
        uid_type uidCls = Label(topic.substr(offsets[2] + 1).c_str()).id();
        msg.dst(uidDst);
        msg.cls(uidCls);
        uid_type uidSrc = Label(jsonObject[AKKA_SRC].as<const
char*>()).id();
        msg.src(uidSrc);

        ActorRef* dst = ActorRef::lookup(uidDst);
        if (dst == 0) {
                WARN(" local Actor : %s not found ", Label::label(uidDst));
                return false;
        }
        ActorRef* src = ActorRef::lookup(uidSrc);
        if (src == 0) { //TODO -> bridge mailbox, no cell, tell & forward
diff
                src = new RemoteActorRef(Label(uidSrc), self());
        }

        for (JsonPair kv : jsonObject) {
                if (strcmp(kv.key().c_str(), AKKA_SRC) == 0) {
                } else {
                        if (kv.value().is<char*>()) {
                                msg(kv.key().c_str(),
kv.value().as<char*>());
                        } else if (kv.value().is<unsigned long>()) {
                                msg(kv.key().c_str(),
kv.value().as<uint64_t>());
                        } else if (kv.value().is<long>()) {
                                msg(kv.key().c_str(),
kv.value().as<int64_t>());
                        } else if (kv.value().is<double>()) {
                                msg(kv.key().c_str(),
kv.value().as<double>());
                        }
                }
        }

//	INFO("%s", msg.toString().c_str());
        return true;
}


bool Bridge::jsonEventToMessage(Msg& msg, std::string& topic, std::string&
message) {

        uint32_t offsets[3] = { 0, 0, 0 };
        uint32_t prevOffset = 0;
        for (uint32_t i = 0; i < 3; i++) {
                uint64_t offset = topic.find('/', prevOffset); // uint64_t
to
support 64 bit architecture ;-)
                if (offset == std::string::npos) break;
                offsets[i] = offset;
                prevOffset = offset + 1;
        }
        if (offsets[0] == 0 || offsets[2] == 0) {
                WARN(" invalid topic : %s", topic.c_str());
                return false;
        }
        uid_type uidSrc = Label(topic.substr(offsets[0] + 1, offsets[2] -
offsets[0]
                                             - 1).c_str()).id();
        uid_type uidCls = Label(topic.substr(offsets[2] + 1).c_str()).id();
        msg.src(uidSrc);
        msg.cls(uidCls);

        _jsonDoc.clear();
        auto rc = deserializeJson(_jsonDoc, message.data());
        if ( ! (rc==DeserializationError::Ok ) ) { // just read this as a
string
                msg(H("value"),message.c_str());
                return true;
        }
        JsonVariant jsonValue = _jsonDoc.as<JsonVariant>();

        if (jsonValue.is<char*>()) {
                msg(H("value"), jsonValue.as<char*>());
        } else if (jsonValue.is<unsigned long>()) {
                msg(H("value"), jsonValue.as<uint64_t>());
        } else if (jsonValue.is<long>()) {
                msg(H("value"), jsonValue.as<int64_t>());
        } else if (jsonValue.is<double>()) {
                msg(H("value"), jsonValue.as<double>());
        } else if (jsonValue.is<bool>()) {
                msg(H("value"), jsonValue.as<bool>());
        } else if (jsonValue.is<JsonObject>()) {
                msg(H("value"), message.c_str());
        } else {
                ERROR(" couldn't handle type : %s",message.c_str());
                return false;
        }

//	INFO("%s", msg.toString().c_str());
        return true;
}*/

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
				    new RemoteActorRef(Label(kv.value().as<char*>()), self());
				msg.src(lr->id());
			} else {
				if (kv.value().is<char*>()) {
					msg(kv.key().c_str(), kv.value().as<char*>());
				} else if (kv.value().is<unsigned long>()) {
					msg(kv.key().c_str(), kv.value().as<uint64_t>());
				} else if (kv.value().is<long>()) {
					msg(kv.key().c_str(), kv.value().as<int64_t>());
				} else if (kv.value().is<double>()) {
					msg(kv.key().c_str(), kv.value().as<double>());
				} else if (kv.value().is<bool>()) {
					msg(kv.key().c_str(), kv.value().as<bool>());
				}
			}
		}
		return true;
	} else {
		JsonVariant jsonValue = _jsonDoc.as<JsonVariant>();
		msg.src(ActorRef::NoSender().id());
		if (!jsonValue.isNull()) {
			if (jsonValue.is<char*>()) {
				msg(H("value"), jsonValue.as<char*>());
			} else if (jsonValue.is<unsigned long>()) {
				msg(H("value"), jsonValue.as<uint64_t>());
			} else if (jsonValue.is<long>()) {
				msg(H("value"), jsonValue.as<int64_t>());
			} else if (jsonValue.is<double>()) {
				msg(H("value"), jsonValue.as<double>());
			} else if (jsonValue.is<bool>()) {
				msg(H("value"), jsonValue.as<bool>());
			} else if (jsonValue.is<JsonObject>()) {
				msg(H("value"), message.c_str());
			} else {
				ERROR(" couldn't handle type : %s", message.c_str());
				return false;
			}
			return true;
		} else {
			ERROR(" not a JsonObject or JsonVariant ? '%s'", message.c_str());
			return false;
		}
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
	    Label(topic.substr(offsets[0] + 1, offsets[2] - offsets[0] - 1).c_str())
	    .id();
	uid_type uidCls = Label(topic.substr(offsets[2] + 1).c_str()).id();
	if (topic.rfind("dst/", 0) == 0) {
		ActorRef* dst = ActorRef::lookup(uid);
		if (dst == 0) {
			WARN(" local Actor : %s not found ", Label::label(uid));
			return false;
		}
		msg.dst(uid);
	} else if (topic.rfind("src/", 0) == 0) {
		ActorRef* src = ActorRef::lookup(uid);
		if (src == 0) { // TODO -> bridge mailbox, no cell, tell & forward diff
			src = new RemoteActorRef(Label(uid), self());
		}
		msg.src(uid);
	}
	msg.cls(uidCls);
	return true;
}

void Bridge::subscribeEventBus() {
	for (auto subscriber : eb.subscribers()) {
		auto cl = subscriber->_classifier;
		DEBUG(" looking up : %s", Label::label(cl.src()));
		ActorRef* ref = ActorRef::lookup(cl.src());
		if (ref != 0 && ref->isLocal() == false) {
			std::string topic = "src/";
			topic += Label::label(cl.src());
			topic += "/";
			topic += Label::label(cl.cls());
			_mqtt.tell(msgBuilder(Mqtt::Subscribe)("topic", topic.c_str()),
			           self());
		}
	}
}
