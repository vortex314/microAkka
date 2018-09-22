#include <Echo.h>
#include <Sender.h>

#define MAX_MESSAGES 100000000

Sender::Sender(va_list args) : startTime(0), str(80) {}
Sender::~Sender() {}

void Sender::preStart() {
    echo = context().system().actorOf<Echo>("echo");
    timers().startPeriodicTimer("PERIODIC_TIMER_1", TimerExpired, 5000);
    anchorSystem = context().system().actorFor("anchor/system");
    context().setReceiveTimeout(100);
}

void Sender::finished() {
    float delta = Sys::millis() - startTime;
    INFO(" '%s' done in %f msec %s ", self().path(), delta, str.c_str());
    INFO(" %f msg/sec ", MAX_MESSAGES * 1000.0 / delta);
}

Receive& Sender::createReceive() {
    return receiveBuilder()
        .match(PONG,
               [this](Envelope& msg) {
                   //			INFO(" PONG received");
                   uint32_t counter;
                   msg.scanf("uS", &counter, &str);
                   if (counter == 0) {
                       startTime = Sys::millis();
                   } else if (counter == MAX_MESSAGES) {
                       finished();
                   }
                   if (counter < MAX_MESSAGES)
                       msg.sender.tell(self(), PING, "us", ++counter, "Hi ");
               })
        .match(TimerExpired,
               [this](Envelope& msg) {
                   UidType key("");
                   uint32_t k;
                   msg.scanf("i", &k);
                   key = k;
                   INFO(" timer expired ! %s ", key.label());
                   //                       timers().cancel("STARTER");
                   echo.tell(self(), PING, "us", 0, "hi!");
                   anchorSystem.tell(
                       self(), "reset", "s",
                       "The quick brown fox jumps over the lazy dog");
               })
        .match(ReceiveTimeout,
               [this](Envelope& msg) {                  
                   INFO(" ReceiveTimeout expired !  ");
               })
        .build();
}

void Sender::handle(Envelope& msg) {
    if (msg.msgClass == PONG) {
        uint32_t counter;
        msg.scanf("uS", &counter, &str);
        if (counter == 0) {
            startTime = Sys::millis();
        } else if (counter == MAX_MESSAGES) {
            finished();
        }
        if (counter < MAX_MESSAGES)
            msg.sender.tell(self(), PING, "us", ++counter, "Hi ");
    }
}
