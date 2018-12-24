#include <Echo.h>
/*
MsgClass Echo::PING = "PING";
MsgClass Echo::PONG = "PONG";
*/
Echo::Echo(va_list args)  {}
Echo::~Echo() {}

Receive& Echo::createReceive() {
	return receiveBuilder()
	       .match(PING,
	[this](Envelope& msg) {
		uint32_t counter;
		assert(msg.get("counter", counter)==0);
		Msg m(PONG,20);
		m.add("counter",counter+1);
		sender().tell(self(),m);
	})
	.build();
}
