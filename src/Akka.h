/*
 * Akka.h
 *
 *  Created on: 2-aug.-2018
 *      Author: Lieven Merckx
 */

#ifndef SRC_AKKA_H_
#define SRC_AKKA_H_
#include <Log.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>
// STL
#include <unordered_map>
#include <vector>
#include <list>
#include <string>
#include <string>
#include <functional>
#include <atomic>
// using namespace std;
// Common
#include <Uid.h>
#include <Xdr.h>
#include <Native.h>

#ifdef PROD
#undef assert
#define assert(xxxx)                                                           \
    if (!(xxxx))                                                               \
    WARN(#xxxx)
#endif

#define AKKA_DST "$DST"
#define AKKA_SRC "$SRC"
#define AKKA_ID "$ID"
#define AKKA_CLS "$CLS"
#define AKKA_TIMER "$timer"

const uid_type UD_DST = H(AKKA_DST);
const uid_type UD_CLS = H(AKKA_CLS);
const uid_type UD_SRC = H(AKKA_SRC);
const uid_type UD_ID = H(AKKA_ID);
const uid_type UD_TIMER = H(AKKA_TIMER);

/*============================================================================
 // Name        : akkaMicro
 // Author      : Lieven
 // Version     :
 // Copyright   : Enjoy the source
 // Description : Akka alike framework in C++ for embedded systems : low RAM
 //
 *
 * Uid  => ActorRef => ActorCell => ActorContext => AbstractActor
 *                                  => Mailbox
 * Uid  => Abs
 * Uid  => MsgClass
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

typedef void (*MsgHandler)(void);
typedef void (*ThreadCode)(void*);

class Receiver;
class Mailbox;
class MessageDispatcher;
class Actor;
class ActorRefFactory;
class ActorContext;
class ActorRef;
class ActorRef;
class ActorSelection;
class ActorSystem;
class ActorCell;
class Message;
class Receive;
class Props;
class Timer;
class Thread;
class ActorMsgBus;
class MsgClass;
class TimerScheduler;
//_____________________________________________________________________ static
extern ActorMsgBus eb;
extern MsgClass SignalFromIsr;

const char* cloneString(const char* s);

//_____________________________________________________________ MsgClass
//
class MsgClass : public Uid {
  public:
    static MsgClass ReceiveTimeout();
    static MsgClass SignalFromIsr();
    //		static  MsgClass TimerExpired();
    static MsgClass PoisonPill();
    static MsgClass AnyClass;
    static MsgClass Properties();
    static MsgClass PropertiesReply();
    MsgClass() : Uid("NONE") {}
    MsgClass(Uid uid) : Uid(uid.id()) {}
    MsgClass(uid_type id) : Uid(id) {}
    MsgClass(const char* name) : Uid(name) {}
    bool operator==(MsgClass b) { return id() == b.id(); }
};
//____________________________________________________________ Msg
//
class Msg : public Xdr {
    enum { OFF_CLS = 1, OFF_SRC = 3, OFF_DST = 5, OFF_ID=7 };

  public:
    Msg();
    Msg(const Msg&);
    ~Msg();
    Msg(MsgClass cls);
    Msg(uint32_t);
    Msg(Uid cls, Uid src);
    Msg& reply(Msg& req);
    Msg& clear();
    template <typename T> Msg& operator()(Uid key, T v) {
        add((uid_type)key.id(), v);
        return *this;
    }

    template <typename T> Msg& operator()(uid_type key, T v) {
        add(key, v);
        return *this;
    }
    Msg& src(uid_type uid);
    Msg& dst(uid_type uid);
    Msg& cls(uid_type uid);
    Msg& id(uint32_t);
    uid_type src();
    uid_type dst();
    uid_type cls();
    const uint32_t id();

    Msg& operator=(const Msg&);
    std::string toString();
};

typedef bool (*MessageMatch)(Msg& msg);
typedef std::function<void(Msg&)> MessageHandler;
typedef std::function<bool(Msg&)> MessageMatcher;

//_________________________________________________________________ Timer
//

class Timer : public Uid, public NativeTimer {
    Msg* _msg;
    bool _autoReload;
    TimerScheduler& _timerScheduler;

  public:
    Timer(Uid key, bool autoReload, uint32_t interval, const Msg& msg,
          TimerScheduler&);
    ~Timer();
    static void callBack(void*);

    bool operator==(Timer&);
    Uid key();
    Msg& msg();
    void msg(const Msg&);
};

//__________________________________________________________ ActorRef

class ActorRef : public Uid {
    static std::unordered_map<uid_type, ActorRef*> _actorRefs;

  public:
    static ActorRef& NoSender();
    static ActorRef* lookup(uid_type id);

    ActorRef(Uid);
    bool operator==(ActorRef&);
    const char* path();

    virtual ~ActorRef();
    virtual void tell(Msg& msg) = 0;
    virtual void tell(Msg& msg, ActorRef& sender) = 0;
    virtual void forward(Msg& message, ActorContext& context) = 0;
    virtual bool isLocal() = 0;
    virtual void signalFromIsr(uint32_t signal) = 0;
};

//______________________________________________________ LocalActorRef
//

class LocalActorRef : public ActorRef {
    ActorCell& _cell;

  public:
    LocalActorRef(Uid, ActorSystem&, Props&, MessageDispatcher&);
    ~LocalActorRef();

    Mailbox& mailbox();
    void mailbox(Mailbox& mailbox);

    void forward(Msg& msg);
    void tell(Msg& msg);
    void tell(Msg& msg, ActorRef& sender);
    void forward(Msg& message, ActorContext& context);
    bool operator==(ActorRef& src);
    bool isLocal();
    void signalFromIsr(uint32_t signal);
    void cell(ActorCell& cell);
    ActorCell& cell();
};

//___________________________________________________ EmptyLocalActorRef
class EmptyLocalActorRef : public ActorRef {
  public:
    EmptyLocalActorRef() : ActorRef(Uid(0, "NoSender")){};
    void tell(Msg& msg){};
    void tell(Msg& msg, ActorRef& src){};
    void forward(Msg& message, ActorContext& context){};
    bool operator==(ActorRef& src) { return false; };
    bool isLocal() { return true; };
    void signalFromIsr(uint32_t){};
};

//__________________________________________________________ TimerScheduler
//

class TimerScheduler {
    std::list<Timer*> _timers;
    ActorRef& _ref;

  public:
    void timerCallback(Timer&);
    TimerScheduler(ActorRef&);
    Timer* find(Uid key);
    Timer* findNextTimeout();
    Uid startPeriodicTimer(Uid key, const Msg&, uint32_t msec);
    Uid startSingleTimer(Uid key, const Msg&, uint32_t msec);
    void cancel(Uid key);
    void cancelAll();
    bool isTimerActive(Uid key);
    ActorRef& ref();
};

//_____________________________________________________________________ Receive
//
class Receive {
    std::list<Receiver*> _receivers;

  public:
    static Receive NullReceive;
    static Receive emptyBehavior;

    Receive();
    Receive& match(MsgClass msgClass, MessageHandler doSome);
    Receive& build();
    Receive& receive(ActorCell&, Msg&);
    Receive& orElse(Receive&);
};

//_____________________________________________________________________
// ActorRefFactory
class ActorRefFactory {
  public:
    /*    virtual ActorRef actorOf(Props props) = 0;
     virtual ActorRef actorOf(Props props, const char* name) = 0;
     virtual ActorSelection actorSelection(ActorPath path) = 0;
     virtual ActorSelection actorSelection(const char* path) = 0;*/
};

//_______________________________________________________________ ActorContext
//
class ActorContext : public ActorRefFactory {

    void systemInvoke(Msg& systemMessage);

  public:
    virtual ActorRef& self() = 0;
    virtual uint32_t receiveTimeout() = 0;
    virtual void setReceiveTimeout(uint32_t msec) = 0;
    void become(Receive& receive) { become(receive, true); }
    virtual void become(Receive& receive, bool discardOld) = 0;
    virtual void unbecome() = 0;
    virtual ActorRef& sender() = 0;
    virtual ActorSystem& system() = 0;
    //		virtual MessageDispatcher& dispatcher() = 0;
    virtual Thread& currentThread() = 0;

    void cancelReceiveTimeout();
    bool hasReceiveTimedOut();
    void resetReceiveTimeout();

    ActorRef& actorFor(const char* name);
    void system(ActorSystem&);

    Receive& receive();
    void receive(Receive&);
    TimerScheduler& timers();
    bool hasTimers();
    static std::list<ActorContext*>& actorContexts();
    void invoke(MessageDispatcher&, Msg&);
    //   ActorRef actorOf(Props props);
    //   ActorRef actorOf(Props props, const char* name);
    //  ActorSelection actorSelection(ActorPath path);
    // ActorSelection actorSelection(const char* path);
};
//________________________________________________________________ ActorCell
//
class ActorCell : public ActorContext {
    Mailbox& _mailbox;
    MessageDispatcher& _dispatcher;
    ActorSystem& _system;
    ActorRef& _self;
    Actor* _actor;

    Thread* _currentThread;
    //		SemaphoreHandle_t _semaphore;

    Receive* _receive;
    Receive* _prevReceive;

    uint32_t _inactivityPeriod;
    uint64_t _lastReceive;
    //		Timer _receiveTimeoutTimer;

    bool _enable;
    TimerScheduler* _timers;

    static std::list<ActorCell*> _actorCells;

  private:
  public:
    ActorCell(ActorSystem&, ActorRef&, MessageDispatcher&, Props&);
    virtual ~ActorCell();

    Mailbox& mailbox();
    ActorSystem& system();
    ActorRef& self();

    void actor(Actor* actor);
    Actor* actor();

    uint32_t receiveTimeout();
    uint64_t lastReceive();
    void setReceiveTimeout(uint32_t msec);

    //		MessageDispatcher& dispatcher();
    Thread& currentThread();
    void currentThread(Thread*);

    void become(Receive& receive, bool discardOld);
    void unbecome();
    void invoke(Msg&);
    void unhandled(Msg&);

    ActorRef& sender();

    static ActorCell* lookup(ActorRef* ref);
    static uint32_t count() { return _actorCells.size(); }

    const char* path();
    void resetReceiveTimeout();
    TimerScheduler& timers();
    bool hasTimers();

    bool hasReceiveTimedOut();
    uint64_t expiresAt();
    static std::list<ActorCell*>& actorCells();

    void sendMessage(Msg& msg);
    void sendMessageFromIsr(Msg& msg);
};

//_________________________________________________________ Actor
//
class Actor {
    ActorCell* _context;
    static std::list<Actor*> _actors;

  public:
    static void timerCallback(ActorRef&, Timer&);
    Actor();
    ~Actor();
    ActorRef& self();
    ActorRef& sender();
    ActorContext& context();
    void context(ActorCell* context);
    virtual void preStart() {}
    void aroundPrestart() {
        WARN("");
        assert(false);
    }
    void postStop() {
        WARN("");
        assert(false);
    }
    void unhandled(Msg& msg);
    virtual Receive& createReceive() = 0;
    TimerScheduler& timers();
    Msg& txdMsg();

    Msg& msgBuilder(MsgClass);
    Msg& replyBuilder(Msg&);

    static Receive& receiveBuilder();
};

//____________________________________________________ RemoteActorRef
//

class RemoteActorRef : public ActorRef {
    ActorRef& _bridge;

  public:
    RemoteActorRef(Uid, ActorRef&);
    void tell(Msg& msg);
    void tell(Msg& msg, ActorRef& src);
    void forward(Msg& message, ActorContext& context);
    bool operator==(ActorRef& src);
    bool isLocal();
    void signalFromIsr(uint32_t signal);
};

//________________________________________________________ ActorSelection

class ActorSelection : public ActorRef {
  public:
    ActorSelection(Uid id);
};

//__________________________________________________________ Thread
//
// per thread data
//
class Thread : public Uid, public NativeThread {
    Msg _txd;
    Msg _rxd;
    MessageDispatcher* _dispatcher;

  public:
    Thread(MessageDispatcher* dispatcher, const char* name, uint32_t stackSize,
           uint32_t priority, void* threadArg, TaskFunction f);
    void currentMessage(Msg* msg);
    Msg& currentMessage();
    Msg& txd();
    Msg& rxd();
    MessageDispatcher& dispatcher();
};

//__________________________________________________________ MessageDispatcher
class MessageDispatcher {
    std::list<ActorCell*> _actorCells;

    uint32_t _threadCount = 1;
    uint32_t _throughput = 10;
    std::list<Thread*> _threads;
    NativeQueue<Mailbox*> _workQueue;

  public:
    MessageDispatcher(uint32_t threadCount = 1, uint32_t stackSize = 1024,
                      uint32_t priority = tskIDLE_PRIORITY + 1);
    void attach(Mailbox&);
    void detach(Mailbox&);
    void attach(ActorCell&);
    void detach(ActorCell&);
    static void handleMailbox(void* thr);
    void start();
    /*		void resume(ActorCell&);
                void suspend(ActorCell&);*/
    void handle(Msg&);
    void unhandled(ActorCell*);
    void nextWakeup(uint64_t t);
    uint64_t nextWakeup();
    Mailbox& mailbox();

    void dispatch(ActorCell&, Msg&);
    void dispatchFromIsr(ActorCell&, Msg&);
    void registerForExecution(Mailbox*, bool hasMessageHint);
    NativeQueue<Mailbox*>& workQueue();
};

//__________________________________________________________ Mailbox
class Mailbox : public NativeQueue<Msg*> {
    ActorCell& _cell;
    std::atomic<uint32_t> _currentStatus;

    static const uint32_t Open = 0;
    static const uint32_t Closed = 1;
    static const uint32_t Scheduled = 2;
    static const uint32_t shouldScheduleMask = 3;
    static const uint32_t shouldNotProcessMask = ~2;
    static const uint32_t suspendMask = ~3;
    static const uint32_t suspendUnit = 4;

  public:
    Mailbox(ActorCell&, uint32_t queueSize);
    int dequeue(Msg&, uint32_t);
    int enqueue(Msg&);
    int enqueueFromIsr(Msg&);
    const char* name();
    void processMailbox(Thread* thread);
    bool shouldProcessMessage();
    bool updateStatus(uint32_t oldStatus, uint32_t newStatus);
    bool canBeScheduledForExecution(bool hasMessageHint);
    bool setAsScheduled();
    bool setAsIdle();
};

//__________________________________________________________ Receiver
//

class Receiver {
    MsgClass _msgClass;
    MessageMatcher _matcher;
    MessageHandler _handler;

  public:
    Receiver(MsgClass msgClass, MessageMatcher matcher, MessageHandler handler);
    Receiver(MsgClass msgClass, MessageHandler handler);
    bool match(Msg& msg);
    void onMessage(Msg& msg);
    static bool alwaysTrue(Msg&) { return true; }
    std::string& toString(std::string& s);
};

//_____________________________________________________________________ Props
//
class Props {
    MessageDispatcher* _dispatcher;

  public:
    Props(MessageDispatcher& d) : _dispatcher(&d) {}
    Props() : _dispatcher(0) {}
    static Props& create() { return *new Props(); }

    Props& withDispatcher(MessageDispatcher& dispatcher) {
        _dispatcher = &dispatcher;
        return *this;
    }

    MessageDispatcher& dispatcher() { return *_dispatcher; }
};
//___________________________________________________________ ActorSystem
class ActorSystem : public Uid {
    Props _defaultProps;
    std::unordered_map<uid_type, ActorRef*> _actorRefs;
    MessageDispatcher& _defaultDispatcher;

  public:
    ActorSystem(Uid, MessageDispatcher& defaultDispatcher);
    Uid uniqueId(const char* name);
    ActorRef& actorFor(const char* address);

    template <class T, typename... Args>
    ActorRef& actorOf(const char* name, Args&&... args) {
        T* actor = new T(args...);
        return *create(actor, name, _defaultProps);
    }

    template <class T> ActorRef& actorOf(Props& props, const char* name, ...) {
        va_list args;
        va_start(args, name);
        T* actor = new T(args);
        va_end(args);

        return *create(actor, name, props);
    }

    void start();

    ActorRef* create(Actor* actor, const char* name, Props& props);
    MessageDispatcher& defaultDispatcher();
    Mailbox& defaultMailbox();
    std::unordered_map<uid_type, ActorRef*>& actorRefs();
};
//____________________________________________________________ Eventbus

template <typename Subscriber, typename Classifier> class SubscriberClassifier {
  public:
    Subscriber _subscriber;
    Classifier _classifier;
    SubscriberClassifier(Subscriber subscriber, Classifier classifier)
        : _subscriber(subscriber), _classifier(classifier) {}
};

//___________________________________________________________ Eventbus
template <typename...> class EventBus;
template <typename Subscriber, typename Classifier, typename Event>
class EventBus<Event, Subscriber, Classifier> {
    std::list<SubscriberClassifier<Subscriber, Classifier>*> _list;

  public:
    void publish(Event& event) {
        Classifier cls = classify(event);
        for (SubscriberClassifier<Subscriber, Classifier>* sc : _list) {
            if (cls == sc->_classifier)
                push(event, sc->_subscriber);
        }
    }

    bool subscribe(Subscriber subscriber, Classifier classifier) {
        _list.push_back(new SubscriberClassifier<Subscriber, Classifier>(
            subscriber, classifier));
        return true;
    }

    bool unsubscribe(Subscriber, Classifier) {
        return false; // TODO not implemented yet
    }

    void unsubscribe(Subscriber) { return; }
    virtual Classifier classify(Event& event) = 0;
    virtual void push(Event& event, Subscriber subscriber) = 0;
    virtual int compareSubscribers(Subscriber a, Subscriber b) = 0;
    virtual ~EventBus() {}

    std::list<SubscriberClassifier<Subscriber, Classifier>*>& subscribers() {
        return _list;
    }
};

class MessageClassifier {
    uid_type _src;
    uid_type _cls;

  public:
    MessageClassifier(uid_type uidSrc, uid_type uidCls) {
        _src = uidSrc;
        _cls = uidCls;
    }

    MessageClassifier(Msg& msg) {
        _src = msg.src();
        _cls = msg.cls();
    }
    MessageClassifier(ActorRef& refSrc, Uid UidCls) {
        _src = refSrc.id();
        _cls = UidCls.id();
    }

    MessageClassifier(Uid UidSrc, Uid UidCls) {
        _src = UidSrc.id();
        _cls = UidCls.id();
    }

    bool operator==(MessageClassifier a) {
        return ((a._src == 0 || _src == 0 || a._src == _src) &&
                (a._cls == 0 || _cls == 0 || a._cls == _cls));
    }
    uid_type src() { return _src; };
    uid_type cls() { return _cls; };
};

class ActorMsgBus : public EventBus<Msg, ActorRef&, MessageClassifier> {
  public:
    void push(Msg& msg, ActorRef& ref) {
        msg.dst(ref.id());
        //			INFO(" event : %s on mailbox : %s
        //",msg.toString().c_str(),ref.Uid());
        ref.tell(msg);
    }
    MessageClassifier classify(Msg& msg) { return MessageClassifier(msg); }
    int compareSubscribers(ActorRef& a, ActorRef& b) {
        return a.id() == b.id();
    }
    ~ActorMsgBus() {}
};

#endif /* SRC_AKKA_H_ */
