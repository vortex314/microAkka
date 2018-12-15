#include <Akka.h>
#ifndef ECHO_H
#define ECHO_H

class Echo : public Actor {
    Str str;

  public:
    static MsgClass PING;
    static MsgClass PONG;

    Echo(va_list args);
    ~Echo();

    Receive& createReceive();
};
#endif
