#ifndef CONFIGACTOR_H
#define CONFIGACTOR_H
#include <Akka.h>
#include <Config.h>

class ConfigActor : public Actor {
		uint64_t startTime;
		uint32_t _counter;
		Uid _startTest;
		Uid _endTest;
		bool _testing;

	public:
		static MsgClass Set;
		static MsgClass Clear;
		static MsgClass Get;

		ConfigActor(va_list args);
		~ConfigActor();

		void preStart();
		Receive& createReceive();

		void handlePong(Envelope& msg);
};

#endif // CONFIGACTOR_H
