#include "Akka.h"
#include <Echo.h>
#include <Mqtt.h>
#include <Bridge.h>
#include <NeuralPid.h>
#include <Sender.h>
#include <System.h>
#include <Publisher.h>
#include <malloc.h>
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

Log logger(1024);
ActorMsgBus eb;

int main() {
	INFO(" MAIN task started");

		Sys::init();
		config.load();
		config.setNameSpace("mqtt");
		std::string url;
		config.get("url", url, "tcp://iot.eclipse.org:1883");
		config.save();

		INFO(" starting microAkka test ");
		static MessageDispatcher defaultDispatcher(1,10240,tskIDLE_PRIORITY+1);
		static ActorSystem actorSystem(Sys::hostname(), defaultDispatcher);

		actorSystem.actorOf<Sender>("sender");
		actorSystem.actorOf<System>("system");
		actorSystem.actorOf<ConfigActor>("config");
		actorSystem.actorOf<NeuralPid>("neuralPid");
		ActorRef& mqtt = actorSystem.actorOf<Mqtt>("mqtt",
				"tcp://limero.ddns.net:1883");
		actorSystem.actorOf<Bridge>("bridge",& mqtt);
		actorSystem.actorOf<Publisher>("publisher",& mqtt);
		defaultDispatcher.start();

	//	defaultDispatcher.unhandled(bridge.cell());

		INFO(" MAIN task ended !! ");
		vTaskStartScheduler();
}
