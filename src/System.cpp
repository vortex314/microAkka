#include "System.h"

#include <System.h>

const MsgClass System::EXIT("EXIT");

System::System(va_list args) : str(80) {}
System::~System() {}

Receive& System::createReceive() {
    return receiveBuilder().match(EXIT, [](Envelope& msg) { exit(0); }).build();
}
