#include <Echo.h>
#include <Sender.h>

Sender::Sender(va_list args) : startTime(0), str(80), _counter(0) {}
Sender::~Sender() {}

void Sender::preStart() {
	echo = context().system().actorOf<Echo>("echo");
	_startTest = timers().startPeriodicTimer("START_TEST", TimerExpired, 3000);
	context().setReceiveTimeout(1000);
}

Receive& Sender::createReceive() {
	return receiveBuilder()
	.match(Echo::PONG, [this](Envelope& msg) { handlePong(msg); })
	.match(TimerExpired,
	[this](Envelope& msg) {
		uint16_t k;
		assert(msg.get(UID_TIMER, k)==0);
		Uid key(k);
		if (key == _startTest) {
			INFO("Start Echo test");
			_endTest = timers().startSingleTimer("END_TEST",
			                                     TimerExpired, 2000);
			startTime = Sys::millis();
			_counter = 0;
			_testing = true;
			Msg ping(Echo::PING,10);
			ping("counter",(uint32_t)0);
			echo.tell(self(),  ping);
		} else if (key == _endTest) {
			float delta = Sys::millis() - startTime;
			_testing = false;
			INFO("End test. %d", _counter);
			INFO(" '%s' done in %f msec %f msg/sec", self().path(),
			     delta, _counter * 1000.0 / delta);
		}
	})
	.match(ReceiveTimeout, [this](Envelope& msg) {INFO(" receiveTimeout ");})
	.build();
}

void Sender::handlePong(Envelope& msg) {
	if (_testing) {
		assert(msg.get("counter", _counter)==0);
		Msg ping(Echo::PING,20);
		ping("counter",_counter);
		sender().tell(self(), ping);
	}
}
