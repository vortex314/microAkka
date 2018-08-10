#include <Akka.h>

Log logger(1024);
/*
 #define ID(cls) H(#cls)

 class DoEcho {
 const static int myId = ID(DoEcho);
 public:
 uint32_t counter;
 const char* message;

 void serialize(Cbor& dst) {
 dst.add(ID(DoEcho));
 dst.addf("us", counter, message);
 }
 bool deserialize(Cbor& src) {
 int id;
 if (src.get(id) && id == ID(DoEcho))
 return src.scanf("us", &counter, &message);
 return false;
 }
 };*/

class Echo: public Actor {
	Str str;

public:
	const static MsgClass DO_ECHO = H("DO_ECHO");
	const static MsgClass DONE_ECHO = H("DONE_ECHO");
//	const static MsgClass DoEchoId = ID(DoEcho);

	Echo(const char* name) :
			Actor(name), str(80) {
	}
	~Echo() {
	}

	void preStart() {
	}
	Receive& createReceive() {
		return receiveBuilder().match(DO_ECHO, [this](Envelope& msg) {
			uint32_t counter;
			msg.scanf("uS", &counter, &str);
			sender().tell(self(), DONE_ECHO, "us", counter,
					"Doe maar een echo");
		}).build();
	}
};
//______________________________________________________________
//
class Sender: public Actor {
	uint64_t startTime;
	Str str;

public:
	Sender(const char* name) :
			Actor(name), startTime(0), str(80) {
	}
	~Sender() {
	}

	Receive& createReceive() {
		return receiveBuilder().match(Echo::DONE_ECHO, [this](Envelope& msg) {
			uint32_t counter;
			msg.scanf("uS", &counter, &str);
			if (counter == 0) {
				startTime = Sys::millis();
			}
			if (counter < 1000000)
			msg.sender.tell(self(), Echo::DO_ECHO, "us", ++counter,"Hi ");
			if (counter == 1000000) {
				INFO(" done in %ld msec %s ",
						Sys::millis() - startTime, str.c_str());
			}
		}).build();
	}
};

Mailbox defaultMailbox(20000, 1000);
ActorSystem actorSystem("system");

int main() {
	INFO(" starting microAkka test ");
	ActorRef echo = actorSystem.actorOf<Echo>("echo");
	ActorRef sender = actorSystem.actorOf<Sender>("sender");

	echo.tell(sender, Echo::DO_ECHO, "us", 0, "hello World");

	sender.mailbox().handleMessages();

	//	actorSystem.loop();
	return 0;
}
