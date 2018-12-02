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
 // Name        : akkaMicro
 // Author      : Lieven
 // Version     :
 // Copyright   : Enjoy the source
 // Description : Akka alike framework in C++ for embedded systems : low RAM
 //
  *
  * UidType => ActorRef => ActorCell => ActorContext => AbstractActor
  *                                  => Mailbox
  * UidType => Abs
  * UidType => MsgClass
  * mailbox N => 1 thread(Dispatcher) 1 => N Receive 1 => 1 Actor
  * 1 ActorRef => 1 mailbox
  *
  * SYSTEM/ACTOR/MSGCLASS
  * dst/system/actor/msgClass for point to point = ["source",id,]
  * src/system/actor/msgClass for event properties
  * dst/esp32/system/set([key,value,key,value]) => setReply([erc])
  * dst/esp32/system/get([key]) => getReply([key,value,key,value])
  * dst/esp32/system/reset() => Nil
  *
tell => mailbox == N = 1 ==> dispatcher =1toN=> actorcells

 //
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
#include <stdarg.h>
#include <stdint.h>

using namespace std;

#include <Cbor.h>
#include <CborQueue.h>
#include <Erc.h>
#include <LinkedList.hpp>
#include <Log.h>
#include <Str.h>
#include <Uid.h>

#define MAX_ACTOR_CELLS 20

typedef void (*MsgHandler)(void);

class Envelope;
class Receiver;
class Mailbox;
class MessageDispatcher;
class Actor;
class AbstractActor;
class ActorRefFactory;
class ActorContext;
class ActorRef;
class ActorRef;
class ActorSelection;
class ActorPath;
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
class Props;

//_____________________________________________________________________ Message

typedef bool (*MsgMatch)(Envelope& msg);
typedef std::function<void(Envelope&)> MessageHandler;
typedef std::function<bool(Envelope&)> MessageMatcher;

extern ActorRef AnyActor;
extern ActorRef NoSender;
extern MsgClass AnyClass;
extern Props defaultProps;
extern Envelope NoMessage; // to handle references cleanly

extern Mailbox defaultMailbox;
extern Mailbox deadLetterMailbox;
extern Mailbox remoteMailbox;
extern ActorMsgBus bus;
extern MsgClass ReceiveTimeout;
extern MsgClass TimerExpired;
extern MsgClass PoisonPill;
extern Receive nullReceive;
extern MessageDispatcher defaultDispatcher;
extern ActorCell noActorCell;

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
    static Receive emptyBehavior;
};

//__________________________________________________________ MessageDispatcher
class MessageDispatcher {
    LinkedList<Mailbox*> _mailboxes;
    LinkedList<ActorCell*> _actorCells;
    ActorCell* _unhandledCell;

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
    void unhandled(ActorCell*);
};
//__________________________________________________________ CoRoutineDispatcher
//
class CoRoutineDispatcher : public MessageDispatcher {};

//_____________________________________________________________________
// ActorRefFactory
class ActorRefFactory {
  public:
    /*    virtual ActorRef actorOf(Props props) = 0;
        virtual ActorRef actorOf(Props props, const char* name) = 0;
        virtual ActorSelection actorSelection(ActorPath path) = 0;
        virtual ActorSelection actorSelection(const char* path) = 0;*/
};

//_______________________________________________________________ AbstractActor
//
/*class AbstractActor {

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
    void ask(ActorRef dest, uint32_t timeout, MsgClass type, const char* format,
             ...);

    virtual Receive& createReceive() = 0;
    Receive& receiveBuilder();
    virtual void preStart();
    virtual void postStop();
    virtual void unhandled(Envelope& msg);

    //	virtual void handle(Envelope& msg){};
    TimerScheduler& timers();
};*/
//__________________________________________________________ ActorRef
class ActorRef {
    UidType _id;
    Mailbox* _mailbox;
    ActorCell* _cell;

    static LinkedList<ActorRef*> _actorRefs;

  public:
    ActorRef();
    ActorRef(UidType id);

    void ask(ActorRef dst, MsgClass type, Envelope& msg, uint32_t timeout);
    void forward(Envelope& msg);
    void tell(ActorRef sender, Envelope& message);
    void tell(ActorRef sender, MsgClass type, const char* format, ...);
    void tell(ActorRef sender, MsgClass type, uint16_t id, const char* format,
              ...);
    void forward(Envelope& message, ActorContext& context);
    Mailbox& mailbox();
    void mailbox(Mailbox& mailbox);
    uid_type id();
    bool operator==(ActorRef& src);
    const char* path();
    static ActorRef noSender;
    static ActorRef* lookup(uid_type id);
    bool isLocal();
    void isLocal(bool b);
	void cell(ActorCell* cell);
	ActorCell* cell();
};
//_______________________________________________________________ ActorContext
class ActorContext : public ActorRefFactory {

    void systemInvoke(Envelope& systemMessage);

  public:
    virtual ActorRef& self() = 0;
    virtual uint32_t receiveTimeout() = 0;
    virtual void setReceiveTimeout(uint32_t msec) = 0;
    void become(Receive& receive) { become(receive, true); };
    virtual void become(Receive& receive, bool discardOld) = 0;
    virtual void unbecome() = 0;
    virtual ActorRef& sender() = 0;
    virtual ActorSystem& system() = 0;

    // ActorContext();

    void cancelReceiveTimeout();
    bool hasReceiveTimedOut();
    void resetReceiveTimeout();

    ActorRef actorFor(const char* name);
    void system(ActorSystem&);

    Receive& receive();
    void receive(Receive&);
    TimerScheduler& timers();
    bool hasTimers();
    static LinkedList<ActorContext*>& actorContexts();
    void invoke(Envelope&);
    //   ActorRef actorOf(Props props);
    //   ActorRef actorOf(Props props, const char* name);
    //  ActorSelection actorSelection(ActorPath path);
    // ActorSelection actorSelection(const char* path);
};
//________________________________________________________________ ActorCell
class ActorCell : public ActorContext {
    Mailbox& _mailbox;
    ActorSystem& _system;
    MessageDispatcher& _dispatcher;
    ActorRef& _self;

    Receive* _receive;
    Actor* _actor;
    Envelope* _currentMessage;

    uint32_t _inactivityPeriod;
    uint64_t _lastReceive;
    bool _enable;
    TimerScheduler* _timers;

    static LinkedList<ActorCell*> _actorCells;

  private:
  public:
    ActorCell(ActorSystem& system, ActorRef& ref, Mailbox& mailbox,
              MessageDispatcher& dispatcher);
    Mailbox& mailbox();
    ActorSystem& system();
    ActorRef& self();
    MessageDispatcher& dispatcher();

    uint32_t receiveTimeout();
    void setReceiveTimeout(uint32_t msec);
    void become(Receive& receive, bool discardOld);
    void unbecome();
    void invoke(Envelope& envelope);

    ActorRef& sender();

    void actor(Actor* actor);
    Actor* actor();
    static ActorCell* cellFor(ActorRef* ref);

    void mailbox(Mailbox&);
    const char* path();
    void resetReceiveTimeout();
    TimerScheduler& timers();
    bool hasTimers();

    bool hasReceiveTimedOut();
    static LinkedList<ActorCell*>& actorCells();
};

//________________________________________________________ ActorRef
/*class ActorRef : public ActorRef {
    ActorCell* _actorCell;

  public:
    ActorRef(UidType id) : ActorRef(id) {
        isLocal(true);
        //   : _actorCell(system, ref, mailbox, dispatcher){};
    };
    void cell(ActorCell* c) { _actorCell = c; };
    ActorCell* cell() { return _actorCell; }
};*/
//_____________________________________________________________________
// Actor
//
class Actor {
    ActorCell* _context;
    static LinkedList<Actor*> _actors;

  public:
    Actor();
    ~Actor();
    ActorRef& self();
    ActorRef& sender();
    ActorContext& context();
    void context(ActorCell* context) { _context = context; }
    void preStart(){};
    void aroundPrestart(){};
    void postStop(){};
    void unhandled(Envelope& msg);
    Receive& createReceive();
    TimerScheduler& timers();

    static Receive& receiveBuilder();
};
class Timers : public Actor {
    TimerScheduler _timers;

  public:
};
//________________________________________________________ ActorSelection

class ActorSelection : public ActorRef {
  public:
    ActorSelection(UidType id);
};

//________________________________________________________ Envelope
class Envelope {
    static uid_type idCounter;

  public:
    ActorRef* sender;
    ActorRef* receiver;
    MsgClass msgClass;
    uint32_t id;
    Cbor message;

    Envelope(uint32_t size);
    Envelope(ActorRef& snd, ActorRef& rcv, MsgClass clz);

    Envelope(ActorRef& snd, ActorRef& rcv, MsgClass clz, uint32_t size);
    Envelope& header(ActorRef& rcv, ActorRef& snd, MsgClass clz);
    Envelope& header(ActorRef& rcv, ActorRef& snd, MsgClass clz, uint16_t id);
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

//_____________________________________________________________________
// Mailbox
class Mailbox : public MessageQueue {
    const char* _name;
    static LinkedList<Mailbox*> _mailboxes;

  public:
    Mailbox(const char* name, uint32_t queueSize, uint32_t messageSize);
    static LinkedList<Mailbox*>& mailboxes();
};

//_____________________________________________________________________
// Receiver
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
//_____________________________________________________________________ Props
//
typedef Actor* (*ActorConstructor)(va_list args);
class Props {
    MessageDispatcher* _dispatcher;
    Mailbox* _mailbox;
    ActorConstructor constructor;

  public:
    Props(MessageDispatcher& d, Mailbox& mb) : _dispatcher(&d), _mailbox(&mb) {}
    static Props& create() {
        return *new Props(defaultDispatcher, defaultMailbox);
    };
    Props& withDispatcher(MessageDispatcher& dispatcher) {
        _dispatcher = &dispatcher;
        return *this;
    };
    Props& withMailbox(Mailbox& mailbox) {
        _mailbox = &mailbox;
        return *this;
    }
    MessageDispatcher& dispatcher() { return *_dispatcher; };
    Mailbox& mailbox() { return *_mailbox; };
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

    ActorRef& actorFor(const char* address) {
        // TODO check local or remote
        ActorRef* ref = new ActorRef(address);
        ref->mailbox(remoteMailbox);
        return *ref;
    }

    template <class T> ActorRef& actorOf(const char* name, ...) {
        va_list args;
        va_start(args, name);
        T* actor = new T(args);
        va_end(args);
        UidType id = ActorSystem::uniqueId(name);
        ActorRef* actorRef = new ActorRef(id);
        actorRef->mailbox(defaultProps.mailbox());
        ActorCell* actorCell =
            new ActorCell(*this, *actorRef, defaultProps.mailbox(),
                          defaultProps.dispatcher());
        actorRef->cell(actorCell);
        actor->context(actorCell);
        actorCell->become(actor->createReceive(), true);
        INFO(" new actor '%s' created", actorRef->path());
        actor->preStart();
        INFO(" actor '%s' preStarted", actorRef->path());
        defaultProps.dispatcher().attach(*actorCell);

        return *actorRef;
    }

    template <class T> ActorRef& actorOf(Props& props, const char* name, ...) {
        va_list args;
        va_start(args, name);
        T* actor = new T(args);
        va_end(args);
        UidType id = ActorSystem::uniqueId(name);
        ActorRef* actorRef = new ActorRef(id);
        actorRef->mailbox(props.mailbox());

        ActorCell* actorCell = new ActorCell(*this, *actorRef, props.mailbox(),
                                             props.dispatcher());
        actorRef->cell(actorCell);
        actor->context(actorCell);
        actorCell->become(actor->createReceive(), true);
        INFO(" new actor '%s' created", actorRef->path());
        actor->preStart();
        INFO(" actor '%s' preStarted", actorRef->path());
        props.dispatcher().attach(*actorCell);
        return *actorRef;
    }
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
/*
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
*/
class Thread {
    uint32_t _signal;

  public:
    void signal(uint32_t signal);
    uint32_t waitSignal(uint32_t msec);
};

#endif /* SRC_AKKA_H_ */
