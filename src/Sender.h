#include <Akka.h>
#ifndef SENDER_H
#define SENDER_H


class Sender : public AbstractActor {
    uint64_t startTime;
    Str str;
    ActorRef echo;
    ActorRef anchorRef;
    uint32_t _counter;

  public:
    Sender(va_list args);
    ~Sender();

    void preStart();
    Receive& createReceive();

    void handlePing(Envelope& msg);
};
#endif