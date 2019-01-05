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

void vAssertCalled( unsigned long ulLine, const char * const pcFileName ) {
	printf("Assert called on : %s:%lu",pcFileName,ulLine);
}

extern "C" void vApplicationMallocFailedHook() {
	WARN(" malloc failed ! ");
	exit(-1);
}



Log logger(1024);


MessageDispatcher defaultDispatcher;


static void  main_task(void *pvParameters) {
	INFO(" MAIN task started");
	MessageDispatcher* dispatcher = (MessageDispatcher*)pvParameters;

	Sys::init();
	config.load();
	config.setNameSpace("mqtt");
	std::string url;
	config.get("url",url,"tcp://iot.eclipse.org:1883");
	config.save();

	INFO(" starting microAkka test ");
	Mailbox defaultMailbox("default", 100); // nbr of messages in queue max
	ActorSystem actorSystem(Sys::hostname(), defaultDispatcher, defaultMailbox);

	ActorRef sender = actorSystem.actorOf<Sender>("sender");
	ActorRef system = actorSystem.actorOf<System>("system");
	ActorRef config = actorSystem.actorOf<ConfigActor>("config");
	ActorRef nnPid = actorSystem.actorOf<NeuralPid>("neuralPid");
	ActorRef mqtt = actorSystem.actorOf<Mqtt>("mqtt", "tcp://limero.ddns.net:1883");
	ActorRef bridge = actorSystem.actorOf<Bridge>("bridge",mqtt);
	ActorRef publisher = actorSystem.actorOf<Publisher>("publisher",mqtt);


	defaultDispatcher.attach(defaultMailbox);
//	defaultDispatcher.unhandled(bridge.cell());

	dispatcher->execute();
	INFO(" MAIN task ended !! ");


}

ActorMsgBus eb;

int main() {


	xTaskCreate(&main_task, "mqtt_task", 10000, &defaultDispatcher,  2, NULL);
	sleep(1);
	vTaskStartScheduler();


}
