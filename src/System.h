#ifndef SYSTEM_H
#define SYSTEM_H

#include <Akka.h>
#include <Config.h>
#include <Mqtt.h>

class System : public Actor {
		Uid _propTimer;
		ActorRef& _mqtt;
	public:
		static const MsgClass Exit;

		System(ActorRef& mqtt);
		virtual ~System();

		Receive& createReceive();
		void preStart();
};

#endif // SYSTEM_H
