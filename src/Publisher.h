#ifndef PUBLISHER_H
#define PUBLISHER_H

#include <Akka.h>

class Publisher: public Actor {
	public:
		Uid _propTimer;
		ActorRef _mqtt;
		std::list<ActorRef*>::iterator _it;
		bool _mqttConnected=false;
	public:
		static const MsgClass Exit;
		static const MsgClass ConfigRequest;
		static const MsgClass PollMe;

		Publisher(va_list args);
		~Publisher();

		Receive& createReceive();
		void preStart();

		ActorRef* nextRef();

};

#endif // PUBLISHER_H
