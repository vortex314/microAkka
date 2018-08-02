#include <Akka.h>

Log logger(1024);

class Echo: public Actor {

public:
	Echo(ActorSystem& system) :
			Actor(system, "echo") {
	}
	~Echo() {
	}

	void preStart() {
	}
	Receive& createReceive() {
		return receiveBuilder().match(AnyActor, H("echo"), [this](Message& msg) {
			msg.sender.tell(self,msg.msgClass,"s","echo");
		}).build();
	}
};
class Wifi: public Actor {
public:
	Wifi(ActorSystem& system) :
			Actor(system, "wifi") {
	}
	~Wifi() {
	}
	Receive& createReceive() {
		return receiveBuilder();
	}
	void onConnected() {
		AnyActor.tell(self, H("connected"), "");
	}
};

int main() {
	Wifi wifi(actorSystem);
	Echo echo(actorSystem);
//	actorSystem.loop();
	return 0;
}
