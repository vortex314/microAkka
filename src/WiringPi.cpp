#ifdef WIRING_PI
#include "WiringPi.h"

const MsgClass WiringPi::GPIO("gpio");


WiringPi::WiringPi(ActorRef& bridge) :_bridge(bridge) {
}

WiringPi::~WiringPi() {
}

void WiringPi::preStart() {
	context().setReceiveTimeout(1000);
}

Receive& WiringPi::createReceive() {
	return receiveBuilder()

	.match(GPIO, [this](Msg& msg) {
		uint32_t pin,writetospi();
		std::string mode,write,read;
		if ( msg.get("pin",pin)==0 ) {
			if ( msg.get("mode",mode)==0 ) {
				if ( mode=="out") {
					pinMode(pin,OUTPUT);
				} else if ( mode=="in") {
					pinMode(pin,INPUT);
				}
			};
			if ( msg.get("write",write)) {
				if ( write ) digitalWrite(pin,HIGH);
				else digitalWrite(pin,LOW);
			}
		}
		uint32_t counter;
		assert(msg.get("counter", counter)==0);
//		INFO("counter:%d",counter);
		sender().tell(msgBuilder(PONG)("counter",counter+1),self());
	}).match(MsgClass::ReceiveTimeout(), [](Msg& msg) {
		INFO(" no messages received recently ! ");

	})
	.match(MsgClass::Properties(), [this](Msg& msg) {
	}).build();
}
#endif