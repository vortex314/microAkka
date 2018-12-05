#include "Akka.h"
#include <Echo.h>
#include <MqttBridge.h>
#include <NeuralPid.h>
#include <Sender.h>
#include <System.h>

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
Mailbox remoteMailbox("remote", 20000, 1000);

int main() {
    Sys::init();

    INFO(" starting microAkka test ");
    //    ActorRef echo = actorSystem.actorOf<Echo>("echo");
    ActorRef sender = actorSystem.actorOf<Sender>("sender");
 //   ActorRef system = actorSystem.actorOf<System>("System");

    ActorRef mqttBridge = actorSystem.actorOf<MqttBridge>(
        Props::create()
            .withMailbox(remoteMailbox)
            .withDispatcher(defaultDispatcher),
        "mqttBridge", "tcp://test.mosquitto.org:1883");
    ActorRef nnPid = actorSystem.actorOf<NeuralPid>("neuralPid");
    ActorSelection ref("pcpav2/ikke");

    defaultDispatcher.attach(defaultMailbox);
    defaultDispatcher.attach(remoteMailbox);
    defaultDispatcher.unhandled(ActorCell::lookup(&mqttBridge));

    while (true) {
        defaultDispatcher.execute();
        millisleep(1);
    };
}
