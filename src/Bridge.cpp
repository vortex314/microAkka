
#include <Bridge.h>
#include <sys/types.h>
#include <unistd.h>
// volatile MQTTAsync_token deliveredtoken;

Bridge::Bridge(va_list args) : _mqtt(NoSender()) {
	_mqtt = va_arg(args,ActorRef) ;
};
Bridge::~Bridge() {}


void Bridge::preStart() {
	timers().startPeriodicTimer("PUB_TIMER", TimerExpired, 5000);
	eb.subscribe(self(), MessageClassifier(_mqtt, Mqtt::Connected));
	eb.subscribe(self(), MessageClassifier(_mqtt, Mqtt::Disconnected));
	eb.subscribe(self(), MessageClassifier(_mqtt, Mqtt::PublishRcvd));
}


Receive& Bridge::createReceive() {
	return receiveBuilder()
	       .match(AnyClass,
	[this](Envelope& msg) {
		if (!(*msg.receiver == self())) {
			INFO(" message received %s:%s:%s  in %s",
			     msg.sender->path(), msg.receiver->path(),
			     msg.msgClass.label(),
			     context().self().path());
			std::string json;
			messageToJson(json, msg);
			std::string topic = "dst/";
			topic += msg.receiver->path();
			Msg m(Mqtt::Publish,100);
			m("topic",topic);
			m("message",json);
			_mqtt.tell(self(),m);
		}
	})
	.match(Mqtt::Connected,
	[this](Envelope& env) {
		INFO(" MQTT CONNECTED");
		_connected=true;
	})
	.match(Mqtt::Disconnected,
	[this](Envelope& env) {
		INFO(" MQTT DISCONNECTED");
		_connected=false;
	})
	.match(Mqtt::PublishRcvd,
	[this](Envelope& env) {
		std::string topic;
		std::string message;
		Msg msg(100);

		if (env.get("topic",topic)==0 && env.get("message",message)==0 &&
		        jsonToMessage(msg,message)) {
			uid_type uid;
			ActorRef* dst,*src;
			if ( msg.get(UID_DST,uid)==0 && (dst=ActorRef::lookup(uid))!=0 && msg.get(UID_SRC,uid)==0 && (src=ActorRef::lookup(uid))!=0) {
				dst->mailbox().enqueue(msg);
			}
			INFO(" processed message %s", message.c_str());
		} else {
			WARN(" processing failed : %s ", message.c_str());
		}
	})
	.match(TimerExpired,
	[this](Envelope& msg) {
		string topic = "src/";
		topic += context().system().label();
		topic += "/system/alive";
		if (_connected) {
			Msg m(Mqtt::Publish,100);
			m("topic",topic)("data","true");
			_mqtt.tell(self(),m);
		}
	})
	.build();
}

bool Bridge::messageToJson(std::string& json, Msg& payload) {
	Tag tag(0);
	payload.rewind();
	_jsonBuffer.clear();
	JsonObject& jsonObject = _jsonBuffer.createObject();

	std::string str;
	while (payload.hasData()) {
		tag.ui32 = payload.peek();
		Uid tagUid=Uid(tag.uid);
		switch (tag.type) {
			case Xdr::BYTES: {
					payload.get(tag.uid,str);
					jsonObject.set(Uid::label(tag.uid),str);
					break;
				}
			case Xdr::UINT: {
					uint64_t u64;
					payload.get(tag.uid,u64);
					if ( tagUid==UID_DST || tagUid==UID_SRC || tagUid==UID_CLS ) {
						jsonObject.set(Uid::label(tag.uid),Uid::label(u64));
					} else {
						jsonObject.set(Uid::label(tag.uid),u64);
					}
					break;
				}
			case Xdr::INT: {
					int64_t i64;
					payload.get(tag.uid,i64);
					jsonObject.set(Uid::label(tag.uid),i64);
					break;
				}
			case Xdr::FLOAT: {
					double d;
					payload.get(tag.uid,d);
					jsonObject.set(Uid::label(tag.uid),d);
					break;
				}
			default:
				{ payload.skip(); }
		}
	};
	jsonObject.printTo(json);
	return true;
}


bool Bridge::jsonToMessage(Msg& msg,std::string& message) {
	_jsonBuffer.clear();
	JsonObject& jsonObject = _jsonBuffer.parseObject((char*)message.c_str());
	if (jsonObject == JsonObject::invalid())
		return false;
	if (!jsonObject.containsKey(AKKA_SRC) || !jsonObject.containsKey(AKKA_DST) || !jsonObject.containsKey(AKKA_CLS))
		return false;

	uid_type uidDst = Uid::add(jsonObject.get<const char*>(AKKA_DST));
	uid_type uidSrc = Uid::add(jsonObject.get<const char*>(AKKA_SRC));
	uid_type uidCls = Uid::add(jsonObject.get<const char*>(AKKA_CLS));
	msg(UID_DST,uidDst);
	msg(UID_SRC,uidSrc);
	msg(UID_CLS,uidCls);

	ActorRef* dst = ActorRef::lookup(uidDst);
	if (dst == 0) {
		dst = &NoSender();
		WARN(" local Actor : %s not found ", Uid::label(uidDst));
	}
	ActorRef* src = ActorRef::lookup(uidSrc);
	if (src == 0) {
		src = new ActorRef(uidSrc, &self().mailbox());
	}

	for (JsonObject::iterator it = jsonObject.begin(); it != jsonObject.end() ; ++it) {
		const char* key=it->key;
		if ( strcmp(key,AKKA_SRC)==0 || strcmp(key,AKKA_DST)==0 || strcmp(key,AKKA_CLS)==0 ) {} else {
			if (it->value.is<char*>()) {
				msg(it->key,it->value.as<char*>());
			} else if (it->value.is<unsigned long>()) {
				msg(it->key,(uint64_t)it->value.as<unsigned long>());
			}  else if (it->value.is<long>()) {
				msg(it->key,(int64_t)it->value.as< long>());
			} else if (it->value.is<double>()) {
				msg(it->key,it->value.as< double>());
			}
		}
	}
	INFO("%s = %s => %s : %s",Uid::label(uidSrc),Uid::label(uidCls),Uid::label(uidDst),message.c_str());
	return true;
}
