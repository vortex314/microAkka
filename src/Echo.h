#include <Akka.h>
#ifndef ECHO_H
#define ECHO_H

// const static MsgClass PING =Uid::hash("PING");
const static MsgClass PONG("PONG");
const static MsgClass PING("PING");

class Echo : public AbstractActor {
    Str str;

  public:
    Echo(va_list args);
    ~Echo();

    void preStart();
    Receive& createReceive();
};
#endif