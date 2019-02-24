#include "Akka.h"
#include <Echo.h>
#include <Mqtt.h>
#include <Bridge.h>
#include <NeuralPid.h>
#include <Sender.h>
#include <System.h>
#include <Publisher.h>
//#include <malloc.h>
#include <ConfigActor.h>

//______________________________________________________________
/*
 * sender : actor to start benchmark of messages send, will also start echo actor
 * system : platform specific
 * nnPid : neural network to simulate PID control
 * bridge : acts as a gateway via mqtt to other actorsystems
 *
 *
 */

void vAssertCalled(unsigned long ulLine, const char * const pcFileName) {
	printf("Assert called on : %s:%lu", pcFileName, ulLine);
}

extern "C" void vApplicationMallocFailedHook() {
	WARN(" malloc failed ! ");
	exit(-1);
}

#define MAX_COUNT 1000000
void test() {
	uint64_t startTime=Sys::millis();
	NativeQueue nq(10,sizeof(void*));
	for(uint32_t count=0;count <MAX_COUNT;count++) {
		Msg msg("msg");
		msg("start",startTime);
		msg("$src","SRC");
		msg("$dst","DST");
		Msg* snd =  new Msg(msg.size());
		*snd=msg;

		nq.send(snd,10);
		nq.send(snd,10);

		Msg msg2;
		Msg* rcv;
		nq.recv((void**)&rcv,10);
		nq.recv((void**)&rcv,10);

		msg2 = *rcv;
		delete rcv;
		std::string arg;
		uint64_t t;
		msg2.get("start",t);
		msg2.get("$src",arg);
		assert(arg=="SRC");
		assert(t==startTime);
	}
	uint64_t endTime = Sys::millis();
	uint64_t delta = endTime-startTime;
	INFO(" delta %lu msec => %.0f msg/sec ",delta,(MAX_COUNT*1000.0)/delta );
}

Log logger(1024);
ActorMsgBus eb;

int main() {
	INFO(" MAIN task started");
	test();

	Sys::init();
	logger.setLogLevel('I');
	config.load();
	config.setNameSpace("mqtt");
	std::string url;
	config.get("url", url, "tcp://iot.eclipse.org:1883");
	config.save();

	INFO(" starting microAkka test ");
	static MessageDispatcher defaultDispatcher(1, 10240, tskIDLE_PRIORITY + 1);
	static ActorSystem actorSystem(Sys::hostname(), defaultDispatcher);

	actorSystem.actorOf<Sender>("sender");
	actorSystem.actorOf<System>("system");
//	actorSystem.actorOf<ConfigActor>("config");
//	actorSystem.actorOf<NeuralPid>("neuralPid");
/*	ActorRef& mqtt =
			actorSystem.actorOf<Mqtt>("mqtt", "tcp://iot.eclipse.org:1883");
	actorSystem.actorOf<Bridge>("bridge", mqtt);
	actorSystem.actorOf<Publisher>("publisher", mqtt);*/
//	defaultDispatcher.start();

	//	defaultDispatcher.unhandled(bridge.cell());

	sleep(20);
	INFO(" MAIN task ended !! ");
//	vTaskStartScheduler();
}
