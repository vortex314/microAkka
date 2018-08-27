#include "/home/lieven/workspace/microAkka/src/Akka.cpp"
#include "/home/lieven/workspace/Common/Log.cpp"
#include "/home/lieven/workspace/Common/Bytes.cpp"
#include "/home/lieven/workspace/Common/Cbor.cpp"
#include "/home/lieven/workspace/Common/Str.cpp"
#include "/home/lieven/workspace/Common/CborQueue.cpp"
#include "/home/lieven/workspace/Common/BipBuffer.cpp"
#include "/home/lieven/workspace/microAkka/src/Sys.cpp"
#include "/home/lieven/workspace/microAkka/src/Uid.cpp"


#define MAX_MESSAGES 10000
const static MsgClass DONE_ECHO("DONE_ECHO");
const static MsgClass DO_ECHO("DO_ECHO");

class Echo: public AbstractActor {
    Str str;

  public:
    Echo() : str(80) {
    }
    ~Echo() {
    }

    void preStart() {
    }
    Receive& createReceive() {
      return receiveBuilder().match(DO_ECHO, [this](Envelope & msg) {
        uint32_t counter;
        msg.scanf("uS", &counter, &str);
        sender().tell(self(), DONE_ECHO, "us", counter,
                      "Give me an echo");
      }).match(("ikke"), [this](Envelope & msg) {
        INFO(" message received %s:%s:%s in %s", msg.sender.path(), msg.receiver.path(), msg.msgClass.name(), context().self().path());
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
      INFO(" '%s' done in %f msec %s ", self().path(), delta, str.c_str());
      INFO(" %f msg/sec ", MAX_MESSAGES * 1000.0 / delta);
    }

    Receive& createReceive() {
      return receiveBuilder().match(DONE_ECHO, [this](Envelope & msg) {
        uint32_t counter;
        msg.scanf("uS", &counter, &str);
        if (counter == 0) {
          startTime = Sys::millis();
        } else if (counter == MAX_MESSAGES) {
          finished();
        }
        if (counter < MAX_MESSAGES)
          msg.sender.tell(self(), DO_ECHO, "us",
                          ++counter, "Hi ");
      }).build();
    }

    void handle(Envelope& msg) {
      if (msg.msgClass == DONE_ECHO) {
        uint32_t counter;
        msg.scanf("uS", &counter, &str);
        if (counter == 0) {
          startTime = Sys::millis();
        } else if (counter == MAX_MESSAGES) {
          finished();
        }
        if (counter < MAX_MESSAGES)
          msg.sender.tell(self(), DO_ECHO, "us", ++counter, "Hi ");
      }
    }
};

Mailbox defaultMailbox(20000, 1000);
ActorSystem actorSystem("system");
Log logger(256);
ActorRef echo = actorSystem.actorOf<Echo>("echo");
ActorRef sender = actorSystem.actorOf<Sender>("sender");


void setup() {
  // initialize digital pin 13 as an output.
  pinMode(2, OUTPUT);
  Serial.begin(115200);
    bus.subscribe(echo, *(new SenderMsgClass(sender, ("ikke"))));
}



// the loop function runs over and over again forever
void loop() {

  digitalWrite(2, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(100);              // wait for a second
  digitalWrite(2, LOW);    // turn the LED off by making the voltage LOW
  delay(100);              // wait for a second

  Envelope env(10);
  env.setHeader(sender, AnyActor, ("ikke"));
  bus.publish(env);

  echo.tell(sender, DO_ECHO, "us", 0, "hello World");

  sender.mailbox().handleMessages();

}


