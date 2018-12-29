#ifndef PUBLISHER_H
#define PUBLISHER_H

#include <Akka.h>
#include <Mqtt.h>

class Publisher: public Actor {
	public:
		Uid _propTimer;
		ActorRef _mqtt;
	public:
		static const MsgClass Exit;
		static const MsgClass ConfigRequest;
		static const MsgClass ConfigReply;

		Publisher(va_list args);
		~Publisher();

		Receive& createReceive();
		void preStart();

};

#endif // PUBLISHER_H
