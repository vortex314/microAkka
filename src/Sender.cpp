#include <Echo.h>
#include <Sender.h>

Sender::Sender()
		: startTime(0), _echo(0),_counter(0),_startTest(""),_endTest("") {
	_testing = false;
}

Sender::~Sender() {
}

void Sender::preStart() {
	_startTest =
			timers().startPeriodicTimer("START_TEST", Msg("StartTest"), 5000);
	context().setReceiveTimeout(1000);
	Label("counter");
	_echo = &context().system().actorOf<Echo>("echo");
}

Receive& Sender::createReceive() {
	return receiveBuilder()

	.match(Echo::PONG, [this](Msg& msg) {handlePong(msg);})

	.match(MsgClass("StartTest"), [this](Msg& msg) {
		INFO("Start Echo test");
		_endTest = timers().startSingleTimer("END_TEST",Msg("EndTest"), 1000);
		startTime = Sys::millis();
		_counter = 0;
		_testing = true;
		_echo->tell(msgBuilder(Echo::PING)("counter",(uint32_t)0),self());
	})

	.match(MsgClass("EndTest"), [this](Msg& msg) {
		float delta = Sys::millis() - startTime;
		_testing = false;
		INFO("End test '%s' done in %f msec for %d msgs =>  %f msg/sec", self().path(),
				delta,_counter, _counter * 1000.0 / delta);
	})

	.match(MsgClass::ReceiveTimeout(), [this](Msg& msg) {
		INFO(" enjoying a timeout ! ");
	})

	.build();
}

void Sender::handlePong(Msg& msg) {
	if (_testing) {
		assert(msg.get(UID("counter"), _counter)==0);
		sender().tell(msgBuilder(Echo::PING)(UID("counter"), _counter), self());
	}
}
