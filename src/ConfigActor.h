#ifndef CONFIGACTOR_H
#define CONFIGACTOR_H
#include <Akka.h>
#include <Config.h>

class ConfigActor : public Actor {
		uint64_t startTime;
		uint32_t _counter;
		Label _startTest;
		Label _endTest;
		bool _testing;

	public:
		static MsgClass Set;
		static MsgClass Clear;
		static MsgClass Get;

		ConfigActor();
		~ConfigActor();

		void preStart();
		Receive& createReceive();

		void handlePong(Msg& msg);
};

#endif // CONFIGACTOR_H
