#include <Echo.h>
Echo::Echo(va_list args) : str(80) {}
Echo::~Echo() {}

void Echo::preStart() {}
Receive& Echo::createReceive() {
    return receiveBuilder()
        .match(PING,
               [this](Envelope& msg) {
                   uint32_t counter;
                   msg.scanf("uS", &counter, &str);
                   sender().tell(self(), PONG, msg.id, "us", counter,
                                 str.c_str());
               })
        .match(("ikke"),
               [this](Envelope& msg) {
                   INFO(" message received %s:%s:%s in %s", msg.sender.path(),
                        msg.receiver.path(), msg.msgClass.label(),
                        context().self().path());
               })
        .build();
}
