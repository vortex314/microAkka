#ifdef WIRING_PI
#include "WiringPi.h"

const MsgClass WiringPi::GPIO("gpio");


WiringPi::WiringPi(ActorRef& bridge) :_bridge(bridge) {
}

WiringPi::~WiringPi() {
}

void WiringPi::preStart() {
	context().setReceiveTimeout(1000);
	  wiringPiSetup () ;
}

Receive& WiringPi::createReceive() {
	return receiveBuilder()

	.match(GPIO, [this](Msg& msg) {
		uint32_t pin,write,rc;
		std::string mode;
		rc=0;
		if ( msg.get("pin",pin)==0 ) {
			INFO(" pin %u",pin);
			if ( msg.get("mode",mode)==0 ) {
				INFO(" mode %s",mode.c_str());
				if ( mode=="out") {
					pinMode(pin,OUTPUT);
				} else if ( mode=="in") {
					pinMode(pin,INPUT);
				}
			};
			if ( msg.get("write",write)==0) {
				INFO(" write %d",write);
				if ( write ) digitalWrite(pin,HIGH);
				else digitalWrite(pin,LOW);
			}
		} else {
			rc = EINVAL;
		}
//		sender().tell(replyBuilder(msg)("rc",rc),self());
	})
	
	.match(MsgClass::ReceiveTimeout(), [](Msg& msg) {
		INFO(" no messages received recently ! ");
	})
	
	.match(MsgClass::Properties(), [this](Msg& msg) {
	}).build();
}
#endif
