#include <Echo.h>
const MsgClass Echo::PONG("PONG");
const MsgClass Echo::PING("PING");

Echo::Echo(va_list args) : str(80) {}
Echo::~Echo() {}

Receive& Echo::createReceive() {
    return receiveBuilder()
        .match(PING,
               [this](Envelope& msg) {
                   uint32_t counter;
                   msg.scanf("uS", &counter, &str);
                   sender().tell(self(), PONG, msg.id, "uuuuS", counter,Uid::count(),ActorRef::count(),ActorCell::count(),str);
               })
        .build();
}
