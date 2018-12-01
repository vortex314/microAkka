#include <Akka.h>
#ifndef ECHO_H
#define ECHO_H

class Echo : public Actor {
    Str str;

  public:
    static const MsgClass PING;
    const static MsgClass PONG;

    Echo(va_list args);
    ~Echo();

    void preStart();
    Receive& createReceive();
};
#endif