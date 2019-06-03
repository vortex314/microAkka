#ifndef WIRINGPI_H
#define WIRINGPI_H
#ifdef WIRING_PI
#include <Akka.h>
#include <wiringPi.h>
class WiringPi: public Actor {

	ActorRef& _bridge;
	
	public:
		static const MsgClass GPIO;
		static const MsgClass I2C;
		static const MsgClass SPI;
		

		WiringPi(ActorRef& bridge);
		~WiringPi();

		void preStart();
		Receive& createReceive();
		void unhandled(Msg& msg);
};
#endif
#endif // WIRINGPI_H
