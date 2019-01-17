#include <Akka.h>
#ifndef SENDER_H
#define SENDER_H
#include <Metric.h>

class Sender : public Actor {
		uint64_t startTime;
		ActorRef& _echo;
		uint32_t _counter=0;
		Uid _startTest;
		Uid _endTest;
		bool _testing;

	public:
		Sender(va_list args);
		~Sender();

		void preStart();
		Receive& createReceive();

		void handlePong(Msg& msg);
};
#endif
