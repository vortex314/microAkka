#include <Akka.h>
#ifndef SENDER_H
#define SENDER_H


class Sender : public AbstractActor {
    uint64_t startTime;
    Str str;
    ActorRef echo;
    ActorRef anchorSystem;

  public:
    Sender(va_list args);
    ~Sender();

    void preStart();
    Receive& createReceive();

    void finished();
    void handle(Envelope& msg);
};
#endif