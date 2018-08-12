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

class Echo : public Actor {
    Str str;

  public:
    const static MsgClass DO_ECHO = H("DO_ECHO");
    const static MsgClass DONE_ECHO = H("DONE_ECHO");
    //	const static MsgClass DoEchoId = ID(DoEcho);

    Echo(const char* name) : Actor(name), str(80) {}
    ~Echo() {}

    void preStart() {}
    Receive& createReceive() {
        return receiveBuilder()
            .match(DO_ECHO,
                   [this](Envelope& msg) {
                       uint32_t counter;
                       msg.scanf("uS", &counter, &str);
                       sender().tell(self(), DONE_ECHO, "us", counter,
                                     "Doe maar een echo");
                   })
            .build();
    }
};
//______________________________________________________________
//
class Sender : public Actor {
    uint64_t startTime;
    Str str;

  public:
    Sender(const char* name) : Actor(name), startTime(0), str(80) {}
    ~Sender() {}

    Receive& createReceive() {
        return receiveBuilder()
            .match(Echo::DONE_ECHO,
                   [this](Envelope& msg) {
                       uint32_t counter;
                       msg.scanf("uS", &counter, &str);
                       if (counter == 0) {
                           startTime = Sys::millis();
                       } else if (counter == MAX_MESSAGES) {
                           float delta = Sys::millis() - startTime;
                           INFO(" done in %f msec %s ", delta, str.c_str());
                           INFO(" %f msg/sec ", MAX_MESSAGES * 1000.0 / delta);
                       }
                       if (counter < MAX_MESSAGES)
                           msg.sender.tell(self(), Echo::DO_ECHO, "us",
                                           ++counter, "Hi ");
                   })
            .build();
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
