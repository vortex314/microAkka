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

#define MAX_MESSAGES 1000000

class Echo: public AbstractActor {
	Str str;

public:
	 const static MsgClass DO_ECHO =H("DO_ECHO");
	 const static MsgClass DONE_ECHO = H("DONE_ECHO");
	//	const static MsgClass DoEchoId = ID(DoEcho);

	Echo() : str(80) {
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
					"Give me an echo");
		}).match(H("ikke"), [this](Envelope& msg) {
			INFO(" ikke message received %d:%d:%d ",msg.sender.id(),msg.receiver.id(),msg.msgClass);
		}).build();
	}
};
//______________________________________________________________
//
class Sender: public AbstractActor {
	uint64_t startTime;
	Str str;

public:
	Sender() : startTime(0), str(80) {
	}
	~Sender() {
	}

	void finished() {
		float delta = Sys::millis() - startTime;
		INFO(" '%s' done in %f msec %s ", self().path(),delta, str.c_str());
		INFO(" %f msg/sec ", MAX_MESSAGES * 1000.0 / delta);
	}

	Receive& createReceive() {
		return receiveBuilder().match(Echo::DONE_ECHO, [this](Envelope& msg) {
			uint32_t counter;
			msg.scanf("uS", &counter, &str);
			if (counter == 0) {
				startTime = Sys::millis();
			} else if (counter == MAX_MESSAGES) {
				finished();
			}
			if (counter < MAX_MESSAGES)
			msg.sender.tell(self(), Echo::DO_ECHO, "us",
					++counter, "Hi ");
		}).build();
	}

	void handle(Envelope& msg) {
		if (msg.msgClass == Echo::DONE_ECHO) {
			uint32_t counter;
			msg.scanf("uS", &counter, &str);
			if (counter == 0) {
				startTime = Sys::millis();
			} else if (counter == MAX_MESSAGES) {
				finished();
			}
			if (counter < MAX_MESSAGES)
				msg.sender.tell(self(), Echo::DO_ECHO, "us", ++counter, "Hi ");
		}
	}
};

Mailbox defaultMailbox(20000, 1000);
ActorSystem actorSystem("system");

int main() {
	Sys::init();
	INFO(" starting microAkka test ");
	ActorRef echo = actorSystem.actorOf<Echo>("echo");
	ActorRef sender = actorSystem.actorOf<Sender>("sender");

	bus.subscribe(echo,*(new SenderMsgClass(sender,H("ikke"))));
	Envelope env(10);
	env.setHeader(sender,echo,H("ikke"));
	bus.publish(env);

	echo.tell(sender, Echo::DO_ECHO, "us", 0, "hello World");

	sender.mailbox().handleMessages();

	//	actorSystem.loop();
	return 0;
}
