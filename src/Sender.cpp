#include <Echo.h>
#include <Sender.h>

Sender::Sender(va_list args) : startTime(0), str(80), _counter(0) {}
Sender::~Sender() {}

void Sender::preStart() {
    echo = context().system().actorOf<Echo>("echo");
    _startTest = timers().startPeriodicTimer("START_TEST", TimerExpired, 20000);
    //    timers().startPeriodicTimer("PERIODIC_TIMER_2", TimerExpired, 20000);

    context().setReceiveTimeout(1000);
}

Receive& Sender::createReceive() {
    return receiveBuilder()
        .match(Echo::PONG, [this](Envelope& msg) { handlePong(msg); })
        .match(TimerExpired,
               [this](Envelope& msg) {
                   uint32_t k;
                   msg.scanf("i", &k);
                   Uid key(k);
                   if (key == _startTest) {
                       INFO("Start Echo test");
                       _endTest = timers().startSingleTimer("END_TEST",
                                                            TimerExpired, 2000);
                       startTime = Sys::millis();
                       _counter = 0;
                       _testing = true;
                       echo.tell(self(), Echo::PING, "us", 0, "hi!");
                   } else if (key == _endTest) {
                       float delta = Sys::millis() - startTime;
                       _testing = false;
                       INFO("End test. %d", _counter);
                       INFO(" '%s' done in %f msec %f msg/sec", self().path(),
                            delta, _counter * 1000.0 / delta);
                   }
               })
        .match(ReceiveTimeout, [this](Envelope& msg) {})
        .build();
}

void Sender::handlePong(Envelope& msg) {
    if (_testing) {
        msg.scanf("u", &_counter);
        sender().tell(self(), Echo::PING, "u", _counter);
    }
}
