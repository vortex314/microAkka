#include "Akka.h"
#include <Echo.h>
#include <MqttBridge.h>
#include <NeuralPid.h>
#include <Sender.h>
#include <System.h>
#include <malloc.h>

//______________________________________________________________
//

Log logger(1024);

void logHeap() {
    struct mallinfo mi;

    mi = mallinfo();
    INFO(" heap size %d ", mi.uordblks);
}

int main() {

    Sys::init();

    INFO(" starting microAkka test ");
    Mailbox defaultMailbox = *new Mailbox("default", 20000, 1000);
    Mailbox remoteMailbox = *new Mailbox("remote", 20000, 1000);
    MessageDispatcher& defaultDispatcher = *new MessageDispatcher();

    ActorSystem actorSystem(Sys::hostname(), defaultDispatcher, defaultMailbox);
    ActorMsgBus eb;

    ActorRef sender = actorSystem.actorOf<Sender>("sender");
    eb.subscribe(sender, MsgClassClassifier(&sender, Echo::PING()));
    Envelope& env = defaultDispatcher.txdEnvelope();
    env.sender = &sender;
    env.msgClass = Echo::PING();
    eb.publish(env);
    //    ActorRef system = actorSystem.actorOf<System>("System");
    ActorRef nnPid = actorSystem.actorOf<NeuralPid>("neuralPid");
    ActorRef mqttBridge = actorSystem.actorOf<MqttBridge>(
        Props::create()
            .withMailbox(remoteMailbox)
            .withDispatcher(defaultDispatcher),
        "mqttBridge", "tcp://test.mosquitto.org:1883");

    defaultDispatcher.attach(defaultMailbox);
    defaultDispatcher.attach(remoteMailbox);
    defaultDispatcher.unhandled(ActorCell::lookup(&mqttBridge));

    while (true) {
        defaultDispatcher.execute();
        if (defaultDispatcher.nextWakeup() >= 0)
            Sys::delay(defaultDispatcher.nextWakeup());
    };
}
