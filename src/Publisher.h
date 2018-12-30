#ifndef PUBLISHER_H
#define PUBLISHER_H

#include <Akka.h>
#include <Mqtt.h>

class Publisher: public Actor {
	public:
		Uid _propTimer;
		ActorRef _mqtt;
		std::list<ActorRef*>::iterator _it;
		bool _mqttConnected=false;
	public:
		static const MsgClass Exit;
		static const MsgClass ConfigRequest;
		static const MsgClass ConfigReply;

		Publisher(va_list args);
		~Publisher();

		Receive& createReceive();
		void preStart();

		ActorRef* nextRef();

};

#endif // PUBLISHER_H
