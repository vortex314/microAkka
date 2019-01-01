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

Log logger(1024);
ActorMsgBus eb;

int main() {

	Sys::init();
	config.load();
	config.setNameSpace("mqtt");
	std::string url;
	config.get("url",url,"tcp://iot.eclipse.org:1883");
	config.save();

	INFO(" starting microAkka test ");
	Mailbox defaultMailbox("default", 100); // nbr of messages in queue max
	MessageDispatcher defaultDispatcher;
	ActorSystem actorSystem(Sys::hostname(), defaultDispatcher, defaultMailbox);

	ActorRef sender = actorSystem.actorOf<Sender>("sender");
	ActorRef system = actorSystem.actorOf<System>("system");
	ActorRef config = actorSystem.actorOf<ConfigActor>("config");
	ActorRef nnPid = actorSystem.actorOf<NeuralPid>("neuralPid");
	ActorRef mqtt = actorSystem.actorOf<Mqtt>("mqtt", "tcp://limero.ddns.net:1883");
	ActorRef bridge = actorSystem.actorOf<Bridge>("bridge",mqtt);
	ActorRef publisher = actorSystem.actorOf<Publisher>("publisher",mqtt);


	defaultDispatcher.attach(defaultMailbox);
	defaultDispatcher.unhandled(bridge.cell());
	defaultDispatcher.execute();

}
