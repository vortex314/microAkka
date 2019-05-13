#include "Publisher.h"
#include <Mqtt.h>

Publisher::Publisher(ActorRef& mqtt)
		: _mqtt(mqtt) {
}

Publisher::~Publisher() {
}

const MsgClass Publisher::Publish("pollMe");

void Publisher::preStart() {
	timers().startPeriodicTimer("publish", Msg("pollTimer"), 1000);
	_it = context().system().actorRefs().begin();
	eb.subscribe(self(), MessageClassifier(_mqtt, Mqtt::Disconnected));
	eb.subscribe(self(), MessageClassifier(_mqtt, Mqtt::Connected));
}

ActorRef* Publisher::nextRef() {
	if (_it == context().system().actorRefs().end()) {
		_it = context().system().actorRefs().begin();
	}
	ActorRef* pa = *_it;
	++_it;
//	INFO(" next ref : %s ",pa->path());
	return pa;
}

void Publisher::publishMsg(Msg& msg) {
	std::string topic;

	msg.rewind();
	while (msg.hasData()) {
		Tag tag(msg.peek());
		Label tag_uid(tag.uid);
		if (tag.uid == UD_DST || tag.uid == UD_SRC || tag.uid == UD_CLS
				|| tag.uid == UD_ID) {
			msg.skip();
		} else {
			topic = "src/";
			topic += sender().path();
			topic += "/";
			topic += tag_uid.label();
			std::string message = "---";
			if (tag.type == Xdr::BYTES) {
				msg.get(tag.uid, message);
			} else if (tag.type == Xdr::UINT) {
				uint64_t ui64;
				msg.get(tag.uid, ui64);
				string_format(message, "%lu", ui64);
			} else if (tag.type == Xdr::INT) {
				int64_t i64;
				msg.get(tag.uid, i64);
				string_format(message, "%ld", i64);
			} else if (tag.type == Xdr::FLOAT) {
				double d;
				msg.get(tag.uid, d);
				string_format(message, "%f", d);
			} else if (tag.type == Xdr::BOOL) {
				bool b;
				msg.get(tag.uid, b);
				b ? message = "true" : message = "false";
			} else {
				msg.skip();
			}
			_mqtt.tell(msgBuilder(Mqtt::Publish)("topic", topic)("message", message), self());
		}

	}

}

Receive& Publisher::createReceive() {
	return receiveBuilder()

	.match(MsgClass::PropertiesReply(), [this](Msg& msg) {
		if ( _mqttConnected == true ) {
			publishMsg(msg);
		}
	})

	.match(MsgClass("pollTimer"), [this](Msg& msg) {
		if ( _mqttConnected == true ) {
			ActorRef* ref=nextRef();
			ref->tell(msgBuilder(MsgClass::Properties()).src(self().id()).dst(ref->id()));
		}
	})

	.match(Publish, [this](Msg& msg) {
		if ( _mqttConnected == true ) {
			publishMsg(msg);
		}
	})

	.match(Mqtt::Connected, [this](Msg& msg) {_mqttConnected=true;})

	.match(Mqtt::Disconnected, [this](Msg& msg) {_mqttConnected=false;})

	.match(MsgClass::Properties(), [this](Msg& msg) {})

	.build();
}
