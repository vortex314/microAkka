#include <Echo.h>

MsgClass Echo::PING = "PING";
MsgClass Echo::PONG = "PONG";

Echo::Echo(va_list args) : str(80) {}
Echo::~Echo() {}

Receive& Echo::createReceive() {
    return receiveBuilder()
        .match(PING,
               [this](Envelope& msg) {
                   uint32_t counter;
                   msg.scanf("u", &counter);
                   sender().tell(self(), PONG, msg.id, "u", counter + 1);
               })
        .build();
}
