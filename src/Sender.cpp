#include <Echo.h>
#include <Sender.h>

Sender::Sender(va_list args) : startTime(0),  _counter(0) {}
Sender::~Sender() {}


void Sender::preStart() {
	echo = context().system().actorOf<Echo>("echo");
	_startTest = timers().startPeriodicTimer("START_TEST",  Msg("StartTest"), 5000);
	context().setReceiveTimeout(1000);
}

Receive& Sender::createReceive() {
	return receiveBuilder()
	.match(Echo::PONG, [this](Envelope& msg) { handlePong(msg); })
	.match(MsgClass("StartTest"),
	[this](Envelope& msg) {
		INFO("Start Echo test");
		_endTest = timers().startSingleTimer("END_TEST",Msg("EndTest"), 1000);
		startTime = Sys::millis();
		_counter = 0;
		_testing = true;

		echo.tell(Msg(Echo::PING)("counter",(uint32_t)0),self());
	})
	.match(MsgClass("EndTest"),
	[this](Envelope& msg) {
		float delta = Sys::millis() - startTime;
		_testing = false;
		INFO("End test. %d", _counter);
		INFO(" '%s' done in %f msec %f msg/sec", self().path(),
		     delta, _counter * 1000.0 / delta);
	})
	.match(ReceiveTimeout(), [this](Envelope& msg) {})
	.build();
}

void Sender::handlePong(Envelope& msg) {
	if (_testing) {
		assert(msg.get("counter", _counter)==0);
		sender().tell(Msg(Echo::PING)("counter",_counter),self() );
	}
}
