#include "Publisher.h"

Publisher::Publisher(va_list args) {
	_mqtt= va_arg(args,ActorRef);
}

Publisher::~Publisher() {
}

void Publisher::preStart() {

}

Receive& Publisher::createReceive() {
	return receiveBuilder()
	       .match(PropertiesReply(),
	[this](Envelope& msg) {
		Msg m(Mqtt::Publish);
		m.add("system/alive","true");
		_mqtt.tell(m,self());
	})
	.build();
}
