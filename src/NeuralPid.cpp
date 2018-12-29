#include "NeuralPid.h"

NeuralPid::NeuralPid(va_list args) : str(100) {
	_topology.push_back(10);
	_topology.push_back(10);
	_topology.push_back(1);
	_pidNet = Net(_topology);
}

NeuralPid::~NeuralPid() {}

void NeuralPid::preStart() {
	timers().startPeriodicTimer("SAMPLE_TIMER_1", TimerExpired(), 1000);
	for (uint32_t i = 0; i < _topology[0]; i++) {
		_inputVals.push_back(0.0);
	}
	_targetVals.push_back(0.0);
}

void NeuralPid::target(double t) { _target = t; }

void NeuralPid::newInput(double m) {
	_inputVals.erase(_inputVals.begin());
	_inputVals.push_back(m);
}

Receive& NeuralPid::createReceive() {
	return receiveBuilder()
	       .match(("sample"),
	[this](Envelope& msg) {
		INFO(" message received %s:%s:%s in %s", msg.sender->path(),
		     msg.receiver->path(), msg.msgClass.label(),
		     context().self().path());
		//
	})
	.match(TimerExpired(),
	[this](Envelope& msg) {
		double v = 0.5;
		newInput(v);
		_pidNet.feedForward(_inputVals);
		_pidNet.getResults(_resultVals);
		_targetVals.clear();
		_targetVals.push_back(_target);
		_pidNet.backProp(_targetVals);
	})
	.build();
}
