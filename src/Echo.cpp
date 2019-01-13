#include <Echo.h>

const MsgClass Echo::PING("ping");
const MsgClass Echo::PONG("pingReply");

Echo::Echo(va_list args)  {}
Echo::~Echo() {}

void Echo::preStart(){
	context().setReceiveTimeout(1000);
}

Receive& Echo::createReceive() {
	return receiveBuilder()
	       .match(PING,
	[this](Msg& msg) {
		uint32_t counter;
		assert(msg.get("counter", counter)==0);
//		INFO("counter:%d",counter);
		sender().tell(msgBuilder(PONG)("counter",counter+1),self());
	})
	.match(MsgClass::ReceiveTimeout(),
		[this](Msg& msg) {
			INFO(" no messages received recently ! ");
		})
	.build();
}

void Echo::unhandled(Msg& msg){
	INFO(" no handler for : %s ",msg.toString().c_str());
}
