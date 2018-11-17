#include <Echo.h>
#include <Sender.h>

#define MAX_MESSAGES 100000

Sender::Sender(va_list args) : startTime(0), str(80), _counter(0) {}
Sender::~Sender() {}

void Sender::preStart() {
    echo = context().system().actorOf<Echo>("echo");
    timers().startPeriodicTimer("PERIODIC_TIMER_1", TimerExpired, 5000);
    context().setReceiveTimeout(1000);
    anchorRef = context().system().actorFor("anchor/system");
}

Receive& Sender::createReceive() {
    return receiveBuilder()
        .match(Echo::PONG, [this](Envelope& msg) { handlePing(msg); })
        .match(TimerExpired,
               [this](Envelope& msg) {
                   UidType key("");
                   uint16_t k;
                   msg.scanf("i", &k);
                   key = k;
                   INFO(" timer expired ! %s ", key.label());
                   //                       timers().cancel("STARTER");
                   INFO(" counter : %d ", _counter);
                   echo.tell(self(), Echo::PING, "us", 0, "hi!");
                   anchorRef.tell(
                       self(), "reset", "s",
                       "The quick brown fox jumps over the lazy dog");
               })
        .match(ReceiveTimeout,
               [this](Envelope& msg) { INFO(" ReceiveTimeout expired !  "); })
        .build();
}

void Sender::handlePing(Envelope& msg) {
    //			INFO(" PONG received");
    msg.scanf("uS", &_counter, &str);
    if (_counter == 0) {
        startTime = Sys::millis();
    } else if (_counter == MAX_MESSAGES) {
        float delta = Sys::millis() - startTime;
        INFO(" '%s' done in %f msec %f msg/sec", self().path(), delta,
             MAX_MESSAGES * 1000.0 / delta);
    }
    if (_counter < MAX_MESSAGES) {
        msg.sender.tell(self(), Echo::PING, "us", ++_counter, "Hi ");
    }
}
