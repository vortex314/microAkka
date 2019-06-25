#include <Echo.h>

const MsgClass Echo::PING("ping");
const MsgClass Echo::PONG("pingReply");

Echo::Echo() {}
Echo::~Echo() {}

void Echo::preStart() { context().setReceiveTimeout(1000); }

Receive& Echo::createReceive() {
    return receiveBuilder()

        .match(PING,
               [this](Msg& msg) {
                   uint32_t counter;
                   assert(msg.get("counter", counter));
                   //		INFO("counter:%d",counter);
                   sender().tell(replyBuilder(msg)("counter", counter + 1),
                                 self());
               })

        .match(MsgClass::ReceiveTimeout(),
               [](Msg& msg) {
                   INFO(" no messages received recently ! ");

               })

        .match(MsgClass::Properties(), [this](Msg& msg) {})

        .build();
}

void Echo::unhandled(Msg& msg) {
    INFO(" no handler for : %s ", msg.toString().c_str());
}
