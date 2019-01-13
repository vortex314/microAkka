#include <Echo.h>

const MsgClass Echo::PING("ping");
const MsgClass Echo::PONG("pingReply");

Echo::Echo(va_list args)  {}
Echo::~Echo() {}

Receive& Echo::createReceive() {
	return receiveBuilder()
	       .match(PING,
	[this](Msg& msg) {
		uint32_t counter;
		assert(msg.get("counter", counter)==0);
//		INFO("counter:%d",counter);
		sender().tell(msgBuilder(PONG)("counter",counter+1),self());
	})
	.build();
}
