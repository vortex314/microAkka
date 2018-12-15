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

    ActorRef sender = actorSystem.actorOf<Sender>("sender");

    ActorRef system = actorSystem.actorOf<System>("System");
    ActorRef nnPid = actorSystem.actorOf<NeuralPid>("neuralPid");
    ActorRef mqtt = actorSystem.actorOf<MqttBridge>(
        Props::create()
            .withMailbox(remoteMailbox)
            .withDispatcher(defaultDispatcher),
        "mqttBridge", "tcp://limero.ddns.net:1883");

    defaultDispatcher.attach(defaultMailbox);
    defaultDispatcher.attach(remoteMailbox);
    defaultDispatcher.unhandled(mqtt.cell());

    /*   eb.subscribe(mqtt, EnvelopeClassifier(wifi, Wifi::Disconnected));
       eb.subscribe(mqtt, EnvelopeClassifier(wifi, Wifi::Connected));
    eb.subscribe(system, EnvelopeClassifier(mqtt, MqttBridge::Connected));
    eb.subscribe(system, EnvelopeClassifier(mqtt, MqttBridge::Disconnected));*/

    ActorMsgBus eb;

    eb.subscribe(sender, EnvelopeClassifier("system/Wifi", Echo::PING));

    while (true) {
        defaultDispatcher.execute();
        if (defaultDispatcher.nextWakeup() > (Sys::millis() + 100)) {
            /*INFO(" sleeping %d ",
                 defaultDispatcher.nextWakeup() - Sys::millis());*/
            Sys::delay(defaultDispatcher.nextWakeup() - Sys::millis());
        }
    };
}
