#include <Bridge.h>
#include <sys/types.h>
#include <unistd.h>
// volatile MQTTAsync_token deliveredtoken;

Bridge::Bridge(ActorRef& mqtt)
	: _mqtt(mqtt) {
	_rxd = 0;
	_txd = 0;
	_connected = false;
}
;
Bridge::~Bridge() {
}

void Bridge::preStart() {
	timers().startPeriodicTimer("pubTimer", Msg("pubTimer"), 5000);
	eb.subscribe(self(), MessageClassifier(_mqtt, Mqtt::Connected));
	eb.subscribe(self(), MessageClassifier(_mqtt, Mqtt::Disconnected));
	eb.subscribe(self(), MessageClassifier(_mqtt, Mqtt::PublishRcvd));
}

Receive& Bridge::createReceive() {
	return receiveBuilder()

	.match(MsgClass::AnyClass, [this](Msg& msg) {
		if (!(msg.dst() == self().id())) {
			std::string message;
			std::string topic;
			messageToJson(topic,message, msg);
//			INFO(" to MQTT : %s=%s",topic.c_str(),message.c_str());
			_mqtt.tell(msgBuilder(Mqtt::Publish)("topic",topic)("message",message),self());
			_txd++;
		}
	})

	.match(Mqtt::Connected, [this](Msg& env) {
		INFO(" MQTT CONNECTED");
		_connected=true;
	})

	.match(Mqtt::Disconnected, [this](Msg& env) {
		INFO(" MQTT DISCONNECTED");
		_connected=false;
	})

	.match(Mqtt::PublishRcvd, [this](Msg& env) {
		std::string topic;
		std::string message;
		Msg msg;
//		INFO("envelope %s",env.toString().c_str());

		if (env.get("topic",topic)==0
		        && env.get("message",message)==0
		        && jsonToMessage(msg,topic,message)) {

			ActorRef* dst,*src;
			if ( msg.dst() ==0 || (dst=ActorRef::lookup(msg.dst()))==0 ) {
				WARN(" dst invalid %u ",msg.dst());
			} else if ( msg.src()==0 && (src=ActorRef::lookup(msg.src()))==0) {
				WARN(" src invalid %u ",msg.src());
			} else {
				dst->tell(msg);
				_rxd++;
//				INFO(" processed message %s", msg.toString().c_str());
			}
		} else {
			WARN(" processing failed : %s ", message.c_str());
		}
	})

	.match(MsgClass("pubTimer"), [this](Msg& msg) {
		std::string topic = "src/";
		topic += context().system().label();
		topic += "/system/alive";
		if (_connected) {
			_mqtt.tell(msgBuilder(Mqtt::Publish)("topic",topic)("data","true"),self());
		}

	})

	.match(MsgClass::Properties(), [this](Msg& msg) {
		sender().tell(replyBuilder(msg)
		              ("txd",_txd)
		              ("rxd",_rxd)
		              ,self());
	})

	.build();
}

bool Bridge::messageToJson(std::string& topic, std::string& message, Msg& msg) {
	topic = "dst/";
	topic += Label::label(msg.dst());
	topic += "/";
	uid_type uid = msg.cls();
	topic += Label(uid).label();

	Tag tag(0);
	msg.rewind();
	_jsonBuffer.clear();
	JsonObject jsonObject = _jsonBuffer.to<JsonObject>();

	std::string str;
	while (msg.hasData()) {
		tag.ui32 = msg.peek();
//		Label tagLabel=Label(tag.uid);
		switch (tag.type) {
			case Xdr::BYTES: {
					msg.getNext(tag.uid, str);
					jsonObject[Label::label(tag.uid)] = (char*) str.c_str(); // cast to char * to enforce copy
					break;
				}
			case Xdr::UINT: {
					uint64_t u64;
					msg.getNext(tag.uid, u64);
					if (tag.uid == UD_DST || tag.uid == UD_SRC
					        || tag.uid == UD_CLS) {
						if (tag.uid == UD_SRC) jsonObject[Label::label(tag.uid)] =
							    Label::label(u64);
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
			default: {
					msg.skip();
				}
		}
	};
	serializeJson(jsonObject, message);
	return true;
}
/*
 *  dst/actorSystem/actor/message_type
 *
 *
 *
 *
 */

bool Bridge::jsonToMessage(Msg& msg, std::string& topic, std::string& message) {
	_jsonBuffer.clear();
	auto error = deserializeJson(_jsonBuffer, message.data());
	JsonObject jsonObject = _jsonBuffer.as<JsonObject>();

	if (error || jsonObject.isNull()) {
		WARN(" Invalid Json : %s ", message.c_str());
		return false;
	}

	if (!jsonObject.containsKey(AKKA_SRC)) {
		WARN(" missing source in json message : %s ", message.c_str());
		return false;
	}
	uint32_t offsets[3] = { 0, 0, 0 };
	uint32_t prevOffset = 0;
	for (uint32_t i = 0; i < 3; i++) {
		uint64_t offset = topic.find('/', prevOffset); // uint64_t to support 64 bit architecture ;-)
		if (offset == std::string::npos) break;
		offsets[i] = offset;
		prevOffset = offset + 1;
	}
	if (offsets[0] == 0 || offsets[2] == 0) {
		WARN(" invalid topic : %s", topic.c_str());
		return false;
	}

//	INFO(" topic : %s %d,%d,%d", topic.c_str(), offsets[0], offsets[1], offsets[2]);
//	INFO(" local actor : %s ", topic.substr(offsets[0] + 1, offsets[2]
//	                                        - offsets[0] - 1).c_str());
//	INFO(" local msg class : %s ", topic.substr(offsets[2] + 1).c_str());

	uid_type uidDst = Label(topic.substr(offsets[0] + 1, offsets[2] - offsets[0]
	                                     - 1).c_str()).id();
	uid_type uidCls = Label(topic.substr(offsets[2] + 1).c_str()).id();
	msg.dst(uidDst);
	msg.cls(uidCls);
	uid_type uidSrc = Label(jsonObject[AKKA_SRC].as<const char*>()).id();
	msg.src(uidSrc);

	ActorRef* dst = ActorRef::lookup(uidDst);
	if (dst == 0) {
		WARN(" local Actor : %s not found ", Label::label(uidDst));
		return false;
	}
	ActorRef* src = ActorRef::lookup(uidSrc);
	if (src == 0) { //TODO -> bridge mailbox, no cell, tell & forward diff
		src = new RemoteActorRef(Label(uidSrc), self());
	}

	for (JsonPair kv : jsonObject) {
		if (strcmp(kv.key().c_str(), AKKA_SRC) == 0) {
		} else {
			if (kv.value().is<char*>()) {
				msg(kv.key().c_str(), kv.value().as<char*>());
			} else if (kv.value().is<unsigned long>()) {
				msg(kv.key().c_str(), kv.value().as<uint64_t>());
			} else if (kv.value().is<long>()) {
				msg(kv.key().c_str(), kv.value().as<int64_t>());
			} else if (kv.value().is<double>()) {
				msg(kv.key().c_str(), kv.value().as<double>());
			}
		}
	}

//	INFO("%s", msg.toString().c_str());
	return true;
}
