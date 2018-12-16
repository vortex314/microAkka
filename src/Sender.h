#include <Akka.h>
#ifndef SENDER_H
#define SENDER_H
#include <Metric.h>

class Sender : public Actor {
    uint64_t startTime;
    Str str;
    ActorRef echo;
    ActorRef anchorRef;
    uint32_t _counter;
    Uid _startTest;
    Uid _endTest;
    bool _testing;

  public:
    Sender(va_list args);
    ~Sender();

    void preStart();
    Receive& createReceive();

    void handlePong(Envelope& msg);
};
#endif