/*
 * Akka.h
 *
 *  Created on: 2-aug.-2018
 *      Author: Lieven Merckx
 */

#ifndef SRC_AKKA_H_
#define SRC_AKKA_H_
#include <Log.h>
#include <Str.h>
#include <Uid.h>
#include <stdio.h>

/*============================================================================
 // Name        : akkaMicro.cpp
 // Author      : Lieven
 // Version     :
 // Copyright   : Enjoy teh source
 // Description : Akka alike framework in C++ for embedded systems : low RAM
 //
 //

tell => mailbox == N = 1 ==> dispatcher =1toN=> actorcells

 // MQTT convention : src/DEVICE/ACTOR/MSGCLASS for events
 // or dst/DEVICE2/ACTOR2/MSGCLASS2 mailbox with sender==DEVICE1/ACTOR1 ,
 // dst/DEVICE2/ACTOR2/DEVICE1/ACTOR1/MSGCLASS
 // dst/steer/angle/master/direction/degrees [1,"abc",2.5,true,null]
 // src/master/vector/value [10.6,3.4,-90,10]
 {"x=m":10.6,"y=m":3.4,"angle=rad":-90,"speed=m/s":10}
 // tell : dst/device ["master/vector","device/steer","angle",23123,-20]
 // tell : dst/device
 ["master/sender","device/uart0","txdBin",23123,"SGVsbG8gd29ybGQgIQ=="]
 // tell : dst/system/actor
 ["master/sender","txdBin",23123,"SGVsbG8gd29ybGQgIQ=="]
 // event : src/device/steer/angle -20
 // event : src/device/actors ["uart0","steer"]
 // Object : long,double,boolean,string,null,array,hashmap
 // tell dst/device/actor
 ["device2/actor2","cmdA",1234,{"x":1234,"y":34.7,"z":33.3}]
 // ActorRef(_id) => ActorCell(_id,_mailbox,_remote) =>
 Mailbox(_queue,_rxd,_txd)
 // Actor(_id,) => ActorContext(ActorCell,_receive,_system, )
 // Mqtt:// mailbox_mqtt  =>
 //
 // mailbox handler reads receiver _id => looks for ActorContext and invokes
 Receive
 // ActorRef -> ActorCell -> mailbox, actor,
 // Actor -> ActorContext -> ActorSystem
 //                       -> Receive -> N Receiver ----> ( MsgClass,filter,
 method ) *						 -> ActorRef
 


 Mailbox -> dispatches message based on ActorRef destination , ActorRef points
 to Mailbox  , points to Receive active
 //
 // 1 mailbox == 1 Thread
 // to avoid overhead of RTTI in embedded systems, a string identifier ( hashed
 )
 // was used to interprete the message format
 //
 //============================================================================*/

#include <functional>
//#include <iostream>
#include <stdarg.h>
#include <stdint.h>
//#include <string.h>

using namespace std;

#include <Cbor.h>
#include <CborQueue.h>
#include <Erc.h>
#include <LinkedList.hpp>
#include <Log.h>
#include <Str.h>
#include <Uid.h>

#define MAX_ACTOR_CELLS 20

//#define MAX_FMT_LEN 22

// ( Actor -> mailbox -> thread )
// all coRoutines == same mailbox, a single thread
//  ActorRef : shorthand UidType reference
//

typedef void (*MsgHandler)(void);

class Envelope;
class Receiver;
class Mailbox;
class MessageDispatcher;
class AbstractActor;
class ActorContext;
class ActorRef;
class ActorSystem;
class ActorCell;
class Envelope;
class Message;
class SystemMessage;
class Receive;
class Props;
class Timer;
class ActorMsgBus;
class MsgClass;
class UidType;
class TimerScheduler;

//_____________________________________________________________________ Message

typedef bool (*MsgMatch)(Envelope& msg);
typedef std::function<void(Envelope&)> MessageHandler;
typedef std::function<bool(Envelope&)> MessageMatcher;

extern const ActorRef AnyActor;
extern const ActorRef NoSender;
extern const MsgClass AnyClass;
extern Envelope NoMessage; // to handle references cleanly

extern Mailbox defaultMailbox;
extern Mailbox deadLetterMailbox;
extern Mailbox remoteMailbox;
extern ActorMsgBus bus;
extern const MsgClass ReceiveTimeout;
extern const MsgClass TimerExpired;
extern const MsgClass PoisonPill;
extern MessageDispatcher defaultDispatcher;

class UidType {
    uid_type _id;

  public:
    UidType(const char* name); // will create entry
    UidType(uint16_t id);      // will not create entry
    bool operator==(UidType v);
    const char* label(); // returns null if not found
                         //    bool hasLabel();
    uid_type id();
    void id(uid_type);
};

class MsgClass : public UidType {

  public:
    MsgClass(const char* name) : UidType(name) {}
};
//_____________________________________________________________________ Timer
//
class Timer : public UidType {

    bool _active;
    bool _periodic;
    uint64_t _expiresAt;
    uint32_t _interval;

  public:
    Timer(UidType key, bool active, bool periodic, uint64_t interval);
    void set(bool active, bool periodic, uint32_t interval);
    bool operator==(Timer&);
    bool active();
    void active(bool);
    uid_type key();
    void load();
    void reload();
    bool expired();
    uint64_t expiresAt();
}; //__________________________________________________________ TimerScheduler
class TimerScheduler {
    LinkedList<Timer*> _timers;

  public:
    TimerScheduler();
    Timer* find(uid_type key);
    Timer* findNextTimeout();
    uid_type startPeriodicTimer(UidType key, MsgClass, uint32_t msec);
    uid_type startSingleTimer(UidType key, MsgClass, uint32_t msec);
    void cancel(uid_type key);
    void cancelAll();
    bool isTimerActive(uid_type key);
};

//_____________________________________________________________________ Receive
//
class Receive {
    LinkedList<Receiver*> _receivers;

  public:
    Receive();
    Receive& match(MsgClass msgClass, MessageHandler doSome);
    Receive& build();
    void onMessage(Envelope&);
    Receive& orElse(Receive&);
};

//__________________________________________________________ MessageDispatcher
class MessageDispatcher {
    LinkedList<Mailbox*> _mailboxes;
    LinkedList<ActorCell*> _actorCells;
    ActorContext* _unhandledContext;

  public:
    MessageDispatcher();
    void attach(Mailbox&);
    void detach(Mailbox&);
    void attach(ActorCell&);
    void detach(ActorCell&);
    void execute();
    void resume(ActorCell&);
    void suspend(ActorCell&);
    void handle(Envelope&);
    void unhandled(ActorContext*);
};
//__________________________________________________________ CoRoutineDispatcher
//
class CoRoutineDispatcher : public MessageDispatcher {};
//_____________________________________________________________________ Actor
//
class Actor {
  public:
    virtual ~Actor();
    virtual ActorRef self() = 0;
    virtual ActorRef sender() = 0;
    virtual ActorContext& context() = 0;
    virtual void preStart() = 0;
    virtual void postStop() = 0;
    virtual void unhandled(Envelope& msg) = 0;
    virtual Receive& createReceive() = 0;
};
//_______________________________________________________________ AbstractActor
//
class AbstractActor : public Actor {

    static LinkedList<AbstractActor*> actors;
    ActorContext* _context;

  public:
    AbstractActor();
    virtual ~AbstractActor();

    ActorRef self();
    ActorRef sender();
    void context(ActorContext* ctx);
    ActorContext& context();
    ActorSystem& system();
    void system(ActorSystem* sys);
    void withDispatcher(MessageDispatcher& dispatcher);

    virtual Receive& createReceive() = 0;
    Receive& receiveBuilder();
    virtual void preStart();
    virtual void postStop();
    virtual void unhandled(Envelope& msg);
    //	virtual void handle(Envelope& msg){};
    TimerScheduler& timers();
};
//__________________________________________________________ ActorRef
class ActorRef : public UidType {

  public:
    ActorRef();
    ActorRef(UidType id);

    //	void ask(ActorRef dst, MsgClass type, Envelope& msg, uint32_t timeout);
    void forward(Envelope& msg);
    void tell(ActorRef sender, Envelope& message);
    void tell(ActorRef sender, MsgClass type, const char* format, ...);
    void tell(ActorRef sender, MsgClass type, uint16_t id, const char* format,
              ...);
    void forward(Envelope& message, ActorContext& context);
    Mailbox& mailbox();
    ActorRef& withMailbox();
    const char* path();
    static ActorRef noSender();
};

//________________________________________________________ ActorSelection

class ActorSelection : public ActorRef {
  public:
    ActorSelection(UidType id);
};

//________________________________________________________________ ActorCell
class ActorCell : public ActorRef {
    Mailbox* _mailbox;
    bool _isLocal;
    ActorContext* _context;

  public:
    ActorCell(UidType id, Mailbox& mailbox, ActorRef& self);
    Mailbox& mailbox();
    void mailbox(Mailbox&);
    void self(ActorRef&);
    ActorRef self();
    const char* path();
};
//_______________________________________________________________ ActorContext
class ActorContext : public ActorCell {
    AbstractActor& _actor;
    ActorSystem& _system;
    Receive* _receive;
    uint32_t _inactivityPeriod;
    uint64_t _lastReceive;
    bool _enable;
    TimerScheduler* _timers;
    Envelope* _currentMessage;

    static LinkedList<ActorContext*> _actorContexts;

    void systemInvoke(Envelope& systemMessage);

  public:
    ActorContext(UidType id, ActorRef self, AbstractActor& actor,
                 ActorSystem& system, Mailbox& mailbox);

    static ActorContext& context(AbstractActor*);
    static ActorContext* context(ActorRef&);

    void become(Receive& receive);
    void unbecome();
    void cancelReceiveTimeout();
    void setReceiveTimeout(uint32_t msec);
    bool hasReceiveTimedOut();
    void resetReceiveTimeout();

    ActorSystem& system();
    ActorRef actorFor(const char* name);
    void system(ActorSystem&);

    ActorRef sender();
    Receive& receive();
    void receive(Receive&);
    TimerScheduler& timers();
    bool hasTimers();
    static LinkedList<ActorContext*>& actorContexts();
    void invoke(Envelope&);
};

//___________________________________________________________ ActorSystem
class ActorSystem : public UidType {
    const char* _name;
    Mailbox* _defaultMailbox;
    Mailbox* _deadLetterMailbox;
    MessageDispatcher* _defaultDispatcher;

  public:
    ActorSystem(const char* name);
    UidType uniqueId(const char* name);

    ActorRef actorFor(const char* address) {
        // TODO check local or remote
        ActorRef ref(address);
        new ActorCell(ref.id(), remoteMailbox, ref);
        return ref;
    }

    template <class T> ActorRef actorOf(const char* name, ...) {
        va_list args;
        va_start(args, name);
        T* actor = new T(args);
        va_end(args);
        UidType id = ActorSystem::uniqueId(name);
        ActorRef* actorRef = new ActorRef(id);
        ActorContext* context =
            new ActorContext(id, *actorRef, *actor, *this, *_defaultMailbox);
        actor->context(context);
        context->receive(actor->createReceive());
        INFO(" new actor '%s' created", actorRef->path());
        actor->preStart();
        INFO(" actor '%s' preStarted", actorRef->path());
        return *actorRef;
    }
};
//_____________________________________________________________________ Envelope
class Envelope {
    static uid_type idCounter;

  public:
    ActorRef sender;
    ActorRef receiver;
    MsgClass msgClass;
    uint32_t id;
    Cbor message;

    Envelope(uint32_t size);
    Envelope(ActorRef snd, ActorRef rcv, MsgClass clz);

    Envelope(ActorRef snd, ActorRef rcv, MsgClass clz, uint32_t size);
    Envelope& header(ActorRef rcv, ActorRef snd, MsgClass clz);
    Envelope& header(ActorRef rcv, ActorRef snd, MsgClass clz, uint16_t id);
    //    bool getHeader();
    static uint32_t newId();
    bool scanf(const char* fmt, ...);
    void addf(const char* fmt, ...);
    void vaddf(const char* fmt, va_list);
    void copyMessage(Envelope& dst);
    bool deserialize(Cbor& message);
    void serialize(Cbor& message);
    Str& toString(Str&);
};

//_____________________________________________________________________
// MessageQueue

class MessageQueue {
    CborQueue _cborQueue;
    Cbor _cbor;

  public:
    MessageQueue(int queueSize, int messageSize);
    bool hasMessages();
    void dequeue(Envelope&);
    void enqueue(Envelope&);
};

//_____________________________________________________________________ Mailbox
class Mailbox : public MessageQueue {
    const char* _name;
    static LinkedList<Mailbox*> _mailboxes;

  public:
    Mailbox(const char* name, uint32_t queueSize, uint32_t messageSize);
    static LinkedList<Mailbox*>& mailboxes();
};

//_____________________________________________________________________ Receiver
//

class Receiver {
    MsgClass _msgClass;
    MessageMatcher _matcher;
    MessageHandler _handler;

  public:
    Receiver(MsgClass msgClass, MessageMatcher matcher, MessageHandler handler);
    Receiver(MsgClass msgClass, MessageHandler handler);
    bool match(Envelope& msg);
    void onMessage(Envelope& msg);
    const static bool alwaysTrue(Envelope&) { return true; }
    Str& toString(Str& s);
};
//______________________________________________________________________
// Eventbus

template <typename Subscriber, typename Classifier> class SubscriberClassifier {
  public:
    Subscriber _subscriber;
    Classifier _classifier;
    SubscriberClassifier(Subscriber subscriber, Classifier classifier)
        : _subscriber(subscriber), _classifier(classifier) {}
};
//______________________________________________________________________
// Eventbus
template <typename...> class EventBus;
template <typename Subscriber, typename Classifier, typename Event>
class EventBus<Event, Subscriber, Classifier> {
    LinkedList<SubscriberClassifier<Subscriber, Classifier>*> _list;

  public:
    void publish(Event event) {
        _list.forEach(
            [&event, this](SubscriberClassifier<Subscriber, Classifier>* sc) {
                if (classify(event) == sc->_classifier)
                    push(event, sc->_subscriber);
            });
    }
    bool subscribe(Subscriber subscriber, Classifier classifier) {
        SubscriberClassifier<Subscriber, Classifier>* sc =
            new SubscriberClassifier<Subscriber, Classifier>(subscriber,
                                                             classifier);
        _list.add(sc);
        return true;
    }
    bool unsubscribe(Subscriber, Classifier) {
        return false; // TOD not implemented yet
    }
    void unsubscribe(Subscriber) { return; }
    virtual Classifier classify(Event event) = 0;
    virtual void push(Event event, Subscriber subscriber) = 0;
    virtual int compareSubscribers(Subscriber a, Subscriber b) = 0;
    virtual ~EventBus() {}
};

class SenderMsgClass {
  public:
    ActorRef _sender;
    MsgClass _msgClass;
    SenderMsgClass(ActorRef sender, MsgClass msgClass)
        : _sender(sender), _msgClass(msgClass) {}

    bool operator==(SenderMsgClass a) {
        return a._sender == _sender && a._msgClass == _msgClass;
    }
};

class ActorMsgBus : public EventBus<Envelope&, ActorRef, SenderMsgClass> {
  public:
    void push(Envelope& envelope, ActorRef ref) {
        envelope.receiver = ref;
        ref.mailbox().enqueue(envelope);
    }
    SenderMsgClass classify(Envelope& envelope) {
        return *(new SenderMsgClass(envelope.sender, envelope.msgClass));
    }
    int compareSubscribers(ActorRef a, ActorRef b) { return 1; }
    ~ActorMsgBus() {}
};

class Thread {
    uint32_t _signal;

  public:
    void signal(uint32_t signal);
    uint32_t waitSignal(uint32_t msec);
};

#endif /* SRC_AKKA_H_ */
