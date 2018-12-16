#include "System.h"

#include <System.h>

const MsgClass System::Exit("Exit");
const MsgClass System::ConfigRequest("ConfigRequest");
const MsgClass System::ConfigReply("ConfigReply");

System::System(va_list args) {}
System::~System() {}

Receive& System::createReceive() {
    return receiveBuilder()
        .match(Exit, [](Envelope& msg) { exit(0); })
        .match(ConfigRequest, [](Envelope& msg) { exit(0); })
        .match(ConfigReply, [](Envelope& msg) { exit(0); })
        .build();
}
