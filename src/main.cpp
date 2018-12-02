#include "Akka.h"
#include <Echo.h>
#include <MqttBridge.h>
#include <NeuralPid.h>
#include <Sender.h>

//______________________________________________________________
//
uint32_t millisleep(uint32_t msec) {
    struct timespec ts;
    ts.tv_sec = msec / 1000;
    ts.tv_nsec = (msec - ts.tv_sec * 1000) * 1000000;
    int erc = nanosleep(&ts, NULL);
    return erc;
};

Log logger(1024);
ActorSystem actorSystem(Sys::hostname());
Mailbox defaultMailbox("default", 20000, 1000);
// Mailbox coRoutineMailbox("coRoutine", 20000, 1000);
Mailbox remoteMailbox("remote", 20000, 1000);
// ActorSelection remoteActor("mqtt://");

int main() {
    Sys::init();

    INFO(" starting microAkka test ");
    //    ActorRef echo = actorSystem.actorOf<Echo>("echo");
    ActorRef sender = actorSystem.actorOf<Sender>("sender");
    ActorRef mqttBridge = actorSystem.actorOf<MqttBridge>(
        Props::create().withMailbox(remoteMailbox), "mqttBridge",
        "tcp://test.mosquitto.org:1883");
    ActorRef nnPid = actorSystem.actorOf<NeuralPid>("neuralPid");
    ActorSelection ref("pcpav2/ikke");

    defaultDispatcher.attach(defaultMailbox);
    defaultDispatcher.attach(remoteMailbox);
	defaultDispatcher.unhandled(ActorCell::cellFor(&mqttBridge));

    while (true) {
        defaultDispatcher.execute();
        millisleep(10);
    };
}
