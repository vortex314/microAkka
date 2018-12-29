#ifndef SYSTEM_H
#define SYSTEM_H

#include <Akka.h>
#include <Config.h>

class System : public Actor {
		Uid _propTimer;
	public:
		static const MsgClass Exit;
		static const MsgClass ConfigRequest;
		static const MsgClass ConfigReply;

		System(va_list args);
		~System();

		Receive& createReceive();
		void preStart();
};

#endif // SYSTEM_H
