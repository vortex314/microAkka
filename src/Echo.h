#ifndef ECHO_H
#define ECHO_H
#include <Akka.h>


class Echo : public Actor {
	
	public:
		enum { PING=H("PING"),PONG=H("PONG") };

		Echo(va_list args);
		~Echo();

		Receive& createReceive();
};
#endif
