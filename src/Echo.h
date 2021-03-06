#ifndef ECHO_H
#define ECHO_H
#include <Akka.h>


class Echo : public Actor {

	public:
		static const MsgClass PING;
		static const MsgClass PONG;

		Echo();
		~Echo();

		void preStart();
		Receive& createReceive();
		void unhandled(Msg& msg);
};
#endif
