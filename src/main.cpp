#include "Akka.h"
#include <MqttBridge.h>

Log logger(1024);

#define MAX_MESSAGES 1000

// const static MsgClass DO_ECHO =Uid::hash("DO_ECHO");
const static MsgClass DONE_ECHO("DONE_ECHO");
const static MsgClass DO_ECHO("DO_ECHO");

ActorSystem actorSystem(Sys::hostname());

class Echo : public AbstractActor {
    Str str;

  public:
    Echo() : str(80) {}
    ~Echo() {}

    void preStart() {}
    Receive& createReceive() {
        return receiveBuilder()
            .match(DO_ECHO,
                   [this](Envelope& msg) {
                       //			INFO(" DO_ECHO called ");
                       uint32_t counter;
                       msg.scanf("uS", &counter, &str);
                       sender().tell(self(), DONE_ECHO, "us", counter,
                                     "Give me an echo");
                   })
            .match(("ikke"),
                   [this](Envelope& msg) {
                       INFO(" message received %s:%s:%s in %s",
                            msg.sender.path(), msg.receiver.path(),
                            msg.msgClass.label(), context().self().path());
                   })
            .build();
    }
};
//______________________________________________________________
//

class Sender : public AbstractActor {
    uint64_t startTime;
    Str str;
    ActorRef echo;
    ActorRef anchorSystem;

  public:
    Sender() : startTime(0), str(80) {}
    ~Sender() {}

    void preStart() {
        echo = actorSystem.actorOf<Echo>("echo");
        timers().startPeriodicTimer("STARTER", TimerExpired, 5000);
        anchorSystem = actorSystem.actorFor("anchor/system");
    }

    void finished() {
        float delta = Sys::millis() - startTime;
        INFO(" '%s' done in %f msec %s ", self().path(), delta, str.c_str());
        INFO(" %f msg/sec ", MAX_MESSAGES * 1000.0 / delta);
    }

    Receive& createReceive() {
        return receiveBuilder()
            .match(DONE_ECHO,
                   [this](Envelope& msg) {
                       //			INFO(" DONE_ECHO received");
                       uint32_t counter;
                       msg.scanf("uS", &counter, &str);
                       if (counter == 0) {
                           startTime = Sys::millis();
                       } else if (counter == MAX_MESSAGES) {
                           finished();
                       }
                       if (counter < MAX_MESSAGES)
                           msg.sender.tell(self(), DO_ECHO, "us", ++counter,
                                           "Hi ");
                   })
            .match(TimerExpired,
                   [this](Envelope& msg) {
                       UidType key("");
                       int k;
                       msg.scanf("i", &k);
                       key=k;
                       INFO(" timer expired ! %s ", key.label());
                       //                       timers().cancel("STARTER");
                       echo.tell(self(), DO_ECHO, "us", 0, "hi!");
//                       anchorSystem.tell(self(), "reset", "i",1);
                   })
            .build();
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

class RemoteMqtt : public AbstractActor {
  public:
    RemoteMqtt() {}
};

uint32_t millisleep(uint32_t msec) {
    struct timespec ts;
    ts.tv_sec = msec / 1000;
    ts.tv_nsec = (msec - ts.tv_sec * 1000) * 1000000;
    int erc = nanosleep(&ts, NULL);
    return erc;
};

Mailbox defaultMailbox("default", 20000, 1000);
Mailbox coRoutineMailbox("coRoutine", 20000, 1000);
Mailbox remoteMailbox("$remote", 20000, 1000);
MessageDispatcher defaultDispatcher;

extern void loop();

int main() {
    Sys::init();
    INFO(" starting microAkka test ");
    //    ActorRef echo = actorSystem.actorOf<Echo>("echo");
    ActorRef sender = actorSystem.actorOf<Sender>("sender");
    ActorRef mqttBridge = actorSystem.actorOf<MqttBridge>("mqttBridge");

    defaultDispatcher.attach(defaultMailbox);
    defaultDispatcher.attach(remoteMailbox);
    defaultDispatcher.attach(*ActorContext::context(sender));
    defaultDispatcher.unhandled(ActorContext::context(mqttBridge));

    while (true) {
        defaultDispatcher.execute();
        millisleep(10);
    };

    //	ActorRef master =
    // defaultActorSystem.actorFor("mqtt://test.mosquitto.org:1883/anchor3/system");

    //	master.tell(sender,"alive","b",true);
    /*
        bus.subscribe(echo, *(new SenderMsgClass(sender, "ikke")));

        for (int i = 0; i < 10; i++) {
            Envelope env(10);
            env.setHeader(sender, AnyActor, ("ikke"));
            bus.publish(env);

            //		echo.tell(sender, DO_ECHO, "us", 0, "hello World");

            sender.mailbox().handleMessages();
        }*/
    //	actorSystem.loop();
    return 0;
}
