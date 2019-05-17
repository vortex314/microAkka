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
	uint64_t startTime = Sys::millis();
	NativeQueue<Msg*> nq(10);
	Msg msg(UID("msg"));

	for (uint32_t count = 0; count < MAX_COUNT; count++) {
		msg.clear();
//		std::shared_ptr<Msg> sp(&msg);
		msg(LABEL("start"), startTime);
		msg(LABEL("$src"), "SRC");
		msg(LABEL("$dst"), "DST");
		Msg* snd = new Msg(msg.size());
		*snd = msg;
		nq.send(snd, 10);

		Msg* rcv;
		nq.recv(&rcv, 10);


		std::string arg;
		uint64_t t;
		rcv->get(UID("start"), t);
		rcv->get(UID("$src"), arg);
		assert(arg == "SRC");
		assert(t == startTime);
		delete rcv;
	}
	uint64_t endTime = Sys::millis();
	uint64_t delta = endTime - startTime;
	INFO(" delta %lu msec => %.0f msg/sec ", delta, (MAX_COUNT*1000.0)/delta);
}

Log logger(1024);
ActorMsgBus eb;

int main() {
	INFO(" MAIN task started");
//	test();
//	exit(1);

	Sys::init();
	logger.setLogLevel('I');
	config.load();
	config.setNameSpace("mqtt");
	std::string url;
	config.get("url", url, "tcp://iot.eclipse.org:1883");
	config.save();

	INFO(" starting microAkka test ");
	static MessageDispatcher defaultDispatcher(2, 10240, tskIDLE_PRIORITY + 1);
	static ActorSystem actorSystem(Sys::hostname(), defaultDispatcher);

	actorSystem.actorOf<Sender>("sender");

//	actorSystem.actorOf<ConfigActor>("config");
//	actorSystem.actorOf<NeuralPid>("neuralPid");
	ActorRef& mqtt =
			actorSystem.actorOf<Mqtt>("mqtt", "tcp://limero.ddns.net:1883");
	actorSystem.actorOf<System>("system", mqtt);
	actorSystem.actorOf<Bridge>("bridge", mqtt);
	actorSystem.actorOf<Publisher>("publisher", mqtt);
//	defaultDispatcher.start();

//	defaultDispatcher.unhandled(bridge.cell());

	Msg hello("hello");

	eb.publish(hello);

	sleep(6000000);
	INFO(" MAIN task ended !! ");
//	vTaskStartScheduler();
}
