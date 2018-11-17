#include "Akka.h"
#include <Echo.h>
#include <MqttBridge.h>
#include <NeuralPid.h>
#include <Sender.h>
#include <etl/endianness.h>

Log logger(1024);
ActorSystem actorSystem(Sys::hostname());
//______________________________________________________________
//
uint32_t millisleep(uint32_t msec) {
    struct timespec ts;
    ts.tv_sec = msec / 1000;
    ts.tv_nsec = (msec - ts.tv_sec * 1000) * 1000000;
    int erc = nanosleep(&ts, NULL);
    return erc;
};

Mailbox defaultMailbox("default", 20000, 1000);
Mailbox coRoutineMailbox("coRoutine", 20000, 1000);
Mailbox remoteMailbox("$remote", 20000, 1000);
MessageDispatcher defaultDispatcher;

int main() {
    Sys::init();
    INFO(" sizeof(int) : %d ", sizeof(size_t));

    INFO(" starting microAkka test ");
    if (etl::endianness::value() == etl::endian::little) {
        INFO(" little endian ");
    }
    if (etl::endianness::value() == etl::endian::big) {
        INFO(" big endian ");
    }
    //    ActorRef echo = actorSystem.actorOf<Echo>("echo");
    ActorRef sender = actorSystem.actorOf<Sender>("sender");
    ActorRef mqttBridge = actorSystem.actorOf<MqttBridge>(
        "mqttBridge", "tcp://test.mosquitto.org:1883");
    ActorRef nnPid = actorSystem.actorOf<NeuralPid>("neuralPid");
    ActorSelection ref("pcpav2/ikke");

    defaultDispatcher.attach(defaultMailbox);
    defaultDispatcher.attach(remoteMailbox);
    defaultDispatcher.attach(*ActorContext::context(sender));
    defaultDispatcher.attach(*ActorContext::context(nnPid));
    defaultDispatcher.unhandled(ActorContext::context(mqttBridge));

    ref.tell(ActorRef::noSender(), MsgClass("NONE"), "i", 1);

    while (true) {
        defaultDispatcher.execute();
        millisleep(10);
    };
}
