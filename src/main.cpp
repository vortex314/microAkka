#include <Akka.h>

Log logger(1024);

class Echo : public Actor {
    Str str;

  public:
    const static MsgClass DO_ECHO = H("DO_ECHO");
    const static MsgClass DONE_ECHO = H("DONE_ECHO");

    Echo(const char* name) : Actor(name), str(80) {}
    ~Echo() {}

    void preStart() {}
    Receive& createReceive() {
        return receiveBuilder()
            .match(DO_ECHO,
                   [this](Message& msg) {
                       uint32_t counter;
                       msg.scanf("uS", &counter, &str);
                       //                       INFO(" received DO_ECHO %d",
                       //                       counter);
                       getSender().tell(self, DONE_ECHO, "us", counter,
                                        "Doe maar een echo");
                   })
            .build();
    }
};

class Sender : public Actor {
    uint64_t startTime;
    Str str;

  public:
    Sender(const char* name) : Actor(name), str(80) {}
    ~Sender() {}

    Receive& createReceive() {
        return receiveBuilder()
            .match(Echo::DONE_ECHO,
                   [this](Message& msg) {
                       uint32_t counter;
                       msg.scanf("uS", &counter, &str);
                       //                      INFO(" received DONE_ECHO %d ",
                       //                      counter);
                       if (counter == 0) {
                           startTime = Sys::millis();
                       }
                       if (counter < 1000000)
                           msg.sender.tell(self, Echo::DO_ECHO, "us", ++counter,
                                           "Hi ");
                       if (counter == 1000000) {
                           INFO(" done in %ld msec %s ",
                                Sys::millis() - startTime, str.c_str());
                       }
                   })
            .build();
    }
};

ActorSystem actorSystem("system", 2000, 1024);

int main() {
	INFO(" starting micorAkka test ");
    ActorRef echo = actorSystem.actorOf<Echo>("echo");
    ActorRef sender = actorSystem.actorOf<Sender>("sender");

    echo.tell(sender, Echo::DO_ECHO, "us", 0, "hello World");

    actorSystem.mailbox.handleMessages();

    //	actorSystem.loop();
    return 0;
}
