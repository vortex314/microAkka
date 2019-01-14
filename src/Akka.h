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
#include <list>
#include <string>
#include <string>
#include <functional>
using namespace std;
// Common
#include <Log.h>
#include <Uid.h>
#include <Xdr.h>
// FreeRTOS
#include <FreeRTOS.h>
#include <semphr.h>
#include <queue.h>
#include <task.h>
#include <timers.h>

#ifdef PROD
#undef assert
#define assert(xxxx) if ( !(xxxx) ) WARN(#xxxx)
#endif

#define AKKA_DST 	"$dst"
#define AKKA_SRC 	"$src"
#define AKKA_ID		"$id"
#define AKKA_CLS	"$cls"
#define AKKA_TIMER 	"$timer"

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

const char* cloneString(const char* s);

//_____________________________________________________________________ Message

//______________________________JUST an IDEA _______________________________ Ref
//
#define UID(x) std::integral_constant<uint16_t, H(x)>::value
#define LABEL(xxx) Label(UID(xxx),xxx)

class Label {
		uid_type _id;
		const char* _label;
	public:
		constexpr Label(const char* label) : _id(H(label)),_label(label) {};
};

class Ref {
		uid_type _id;
		static std::unordered_map<uid_type, void*> _refs;
	public:
		Ref(uid_type uid, void *object) {
			if (_refs.find(uid) == _refs.end()) {
				_refs.emplace(_id, (void*) object);
			}
		}
		Ref(uid_type uid, const char* label) {
			if (_refs.find(uid) == _refs.end()) {
				char* object = new char[strlen(label) + 1];
				strcpy(object, label);
				_refs.emplace(_id, (void*) object);
			}
		}
		~Ref(); // the object destruction should lead to Ref lookup destruction
		inline uid_type id() {
			return _id;
		}
		;

		bool operator==(Ref& dst) {
			return _id == dst._id;
		}

};

//_____________________________________________________________ MsgClass
//
class MsgClass: public Uid {
	public:
		static MsgClass ReceiveTimeout();
//		static  MsgClass TimerExpired();
		static MsgClass PoisonPill();
		static MsgClass AnyClass();
		static MsgClass Properties();
		static MsgClass PropertiesReply();

		MsgClass()
				: Uid("NONE") {
		}
		MsgClass(uid_type id)
				: Uid(id) {
		}
		MsgClass(const char* name)
				: Uid(name) {
		}
		bool operator==(MsgClass b) {
			return id() == b.id();
		}
		;
};
//____________________________________________________________ Msg
//
class Msg: public Xdr {
		enum {
			OFF_CLS = 1, OFF_SRC = 3, OFF_DST = 5, OFF_ID
		};
	public:
		Msg();
		~Msg();
		Msg(MsgClass cls);
		Msg(uint32_t);
		Msg(Uid cls, Uid src);
		Msg& reply(Msg& req);
		Msg& clear();
		template<typename T> Msg& operator()(Uid key, T v) {
			add((uid_type) key.id(), v);
			return *this;
		}
		template<typename T> Msg& operator()(uid_type key, T v) {
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
		uint32_t id();

		std::string toString();
		Msg& operator=(const Msg&);
};

typedef bool (*MessageMatch)(Msg& msg);
typedef std::function<void(Msg&)> MessageHandler;
typedef std::function<bool(Msg&)> MessageMatcher;

//_________________________________________________________________ Timer
//
class Timer: public Uid {
		Msg* _msg;
		TimerHandle_t _timer;
		bool _autoReload;
		TimerScheduler& _timerScheduler;
		uint32_t _interval;
		uint64_t _lastReset;

	public:
		Timer(Uid key, bool autoReload, uint32_t interval, const Msg& msg,
				TimerScheduler&);
		~Timer();
		static void callBack(TimerHandle_t);
		void start();
		void stop();
		bool operator==(Timer&);
		Uid key();

		void interval(uint32_t);
		uint32_t interval();
		void reset();
		Msg& msg();
		void msg(const Msg&);
};
//__________________________________________________________ ActorRef
class ActorRef: public Uid {
		Mailbox* _mailbox;
		ActorCell* _cell;

		static unordered_map<uid_type, ActorRef*>* _actorRefs;
		static unordered_map<uid_type, ActorRef*>* actorRefs() {
			if (_actorRefs == 0) {
				_actorRefs = new unordered_map<uid_type, ActorRef*>();
			}
			return _actorRefs;
		}

	public:

		static ActorRef& NoSender();
		ActorRef();
		ActorRef(Uid id);
		ActorRef(Uid id, Mailbox* mb);

		void ask(ActorRef dst, MsgClass type, Msg& msg, uint32_t timeout);
		void forward(Msg& msg);
		void tell(Msg& msg);
		void tell(Msg& msg, ActorRef sender);
//		void tell(ActorRef& sender, MsgClass type, uint16_t id, const char* format,
//		...);
		void forward(Msg& message, ActorContext& context);
		Mailbox& mailbox();
		void mailbox(Mailbox& mailbox);
		bool operator==(ActorRef src);
		const char* path();
		static ActorRef* lookup(Uid id);
		static uint32_t count() {
			return actorRefs()->size();
		}
		bool isLocal();
		void isLocal(bool b);
		void cell(ActorCell* cell);
		ActorCell* cell();
};
//__________________________________________________________ TimerScheduler
class TimerScheduler {
		list<Timer*> _timers;
		ActorRef _ref;

	public:
		void timerCallback(Timer&);
		TimerScheduler(ActorRef);
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
		list<Receiver*> _receivers;

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
class ActorContext: public ActorRefFactory {

		void systemInvoke(Msg& systemMessage);

	public:
		virtual ActorRef& self() = 0;
		virtual uint32_t receiveTimeout() = 0;
		virtual void setReceiveTimeout(uint32_t msec) = 0;
		void become(Receive& receive) {
			become(receive, true);
		}
		;
		virtual void become(Receive& receive, bool discardOld) = 0;
		virtual void unbecome() = 0;
		virtual ActorRef& sender() = 0;
		virtual ActorSystem& system() = 0;
//		virtual MessageDispatcher& dispatcher() = 0;
		virtual Thread& currentThread()=0;

		void cancelReceiveTimeout();
		bool hasReceiveTimedOut();
		void resetReceiveTimeout();

		ActorRef actorFor(const char* name);
		void system(ActorSystem&);

		Receive& receive();
		void receive(Receive&);
		TimerScheduler& timers();
		bool hasTimers();
		static list<ActorContext*>& actorContexts();
		void invoke(MessageDispatcher&, Msg&);
		//   ActorRef actorOf(Props props);
		//   ActorRef actorOf(Props props, const char* name);
		//  ActorSelection actorSelection(ActorPath path);
		// ActorSelection actorSelection(const char* path);
};
//________________________________________________________________ ActorCell
class ActorCell: public ActorContext {
		Mailbox& _mailbox;
		ActorSystem& _system;
		ActorRef& _self;
		Thread* _currentThread;
		SemaphoreHandle_t _semaphore;

		Receive* _receive;
		Receive* _prevReceive;
		Actor* _actor;

		uint32_t _inactivityPeriod;
		uint64_t _lastReceive;
		Timer* _receiveTimer;
		bool _enable;
		TimerScheduler* _timers;

		static list<ActorCell*> _actorCells;

	private:
	public:
		ActorCell(ActorSystem& system, ActorRef& ref, Mailbox& mailbox, Actor*);
		~ActorCell();
		Mailbox& mailbox();
		ActorSystem& system();
		ActorRef& self();
//		MessageDispatcher& dispatcher();
		Thread& currentThread();
		void currentThread(Thread*);
		uint32_t receiveTimeout();
		void setReceiveTimeout(uint32_t msec);
		void startReceiveTimeout();
		void become(Receive& receive, bool discardOld);
		void unbecome();
		void invoke(Msg&);
		void unhandled(Msg&);

		ActorRef& sender();

		void actor(Actor* actor);
		Actor* actor();
		static ActorCell* lookup(ActorRef* ref);
		static uint32_t count() {
			return _actorCells.size();
		}

		void mailbox(Mailbox&);
		const char* path();
		void resetReceiveTimeout();
		TimerScheduler& timers();
		bool hasTimers();

		bool hasReceiveTimedOut();
		uint64_t expiresAt();
		static list<ActorCell*>& actorCells();
};

//_________________________________________________________ Actor
//
class Actor {
		ActorCell* _context;
		static list<Actor*> _actors;

	public:
		static void timerCallback(ActorRef&, Timer&);
		Actor();
		~Actor();
		ActorRef& self();
		ActorRef& sender();
		ActorContext& context();
		void context(ActorCell* context) {
			_context = context;
		}
		virtual void preStart() {
		}
		;
		void aroundPrestart() {
			WARN("");
			assert(false);
		}
		;
		void postStop() {
			WARN("");
			assert(false);
		}
		;
		void unhandled(Msg& msg);
		virtual Receive& createReceive() = 0;
		TimerScheduler& timers();
		Msg& txdMsg();

		Msg& msgBuilder(MsgClass);
		Msg& replyBuilder(Msg&);

		static Receive& receiveBuilder();
};

//________________________________________________________ ActorSelection

class ActorSelection: public ActorRef {
	public:
		ActorSelection(Uid id);
};

//__________________________________________________________ Thread
//
// per thread data
//
class Thread: public Ref {
		uint32_t _stackSize = 1024;
		uint32_t _priority = tskIDLE_PRIORITY + 1;
		Msg _txd;
		Msg _rxd;
		TaskHandle_t _task;
		MessageDispatcher* _dispatcher;
		std::string _name;
	public:
		Thread(MessageDispatcher* dispatcher, const char* name,
				uint32_t stackSize = 1024,
				uint32_t priority = tskIDLE_PRIORITY + 1);
		void currentMessage(Msg* msg);
		Msg& currentMessage();
		Msg& txd();
		Msg& rxd();
		void start(ThreadCode);
		MessageDispatcher& dispatcher();
		const char* name();

};

//__________________________________________________________ MessageDispatcher
class MessageDispatcher {
		Mailbox* _mailbox;
//		list<Mailbox*> _mailboxes;
		list<ActorCell*> _actorCells;
		ActorCell* _unhandledCell;

		uint32_t _threadCount = 1;

//		uint32_t _throughput;
		list<Thread*> _threads;

	public:
		MessageDispatcher(uint32_t threadCount = 1, uint32_t stackSize = 1024,
				uint32_t priority = tskIDLE_PRIORITY + 1);
		void attach(Mailbox&);
		void detach(Mailbox&);
		void attach(ActorCell&);
		void detach(ActorCell&);
		static void handleMailbox(void* thr);
		void start();
		void resume(ActorCell&);
		void suspend(ActorCell&);
		void handle(Msg&);
		void unhandled(ActorCell*);
		void nextWakeup(uint64_t t);
		uint64_t nextWakeup();
		Mailbox& mailbox();

};

//__________________________________________________________ Mailbox
class Mailbox {
		QueueHandle_t _queue;
		const char* _name;

	public:
		Mailbox(const char* name, uint32_t queueSize);
		int dequeue(Msg&, uint32_t);
		int enqueue(Msg&);
		const char* name();
};

//__________________________________________________________ Receiver
//

class Receiver {
		MsgClass _msgClass;
		MessageMatcher _matcher;
		MessageHandler _handler;

	public:
		Receiver(MsgClass msgClass, MessageMatcher matcher,
				MessageHandler handler);
		Receiver(MsgClass msgClass, MessageHandler handler);
		bool match(Msg& msg);
		void onMessage(Msg& msg);
		static bool alwaysTrue(Msg&) {
			return true;
		}
		string& toString(string& s);
};

//_____________________________________________________________________ Props
//
class Props {
		MessageDispatcher* _dispatcher;
		Mailbox* _mailbox;

	public:
		Props(MessageDispatcher& d, Mailbox& mb)
				: _dispatcher(&d), _mailbox(&mb) {
		}
		Props()
				: _dispatcher(0), _mailbox(0) {
		}
		static Props& create() {
			return *new Props();
		}
		;
		Props& withDispatcher(MessageDispatcher& dispatcher) {
			_dispatcher = &dispatcher;
			return *this;
		}
		;
		Props& withMailbox(Mailbox& mailbox) {
			_mailbox = &mailbox;
			return *this;
		}
		MessageDispatcher& dispatcher() {
			return *_dispatcher;
		}
		;
		Mailbox& mailbox() {
			return *_mailbox;
		}
		;
};
//___________________________________________________________ ActorSystem
class ActorSystem: public Uid {
		const char* _name;
		Props _defaultProps;
		list<ActorRef*> _actorRefs;

	public:
		ActorSystem(const char* name, MessageDispatcher& defaultDispatcher,
				Mailbox& defaultMailbox);
		Uid uniqueId(const char* name);
		ActorRef& actorFor(const char* address);

		template<class T> ActorRef& actorOf(const char* name, ...) {
			va_list args;
			va_start(args, name);
			T* actor = new T(args);
			va_end(args);

			return *create(actor, name, _defaultProps);
		}

		template<class T> ActorRef& actorOf(Props& props, const char* name,
				...) {
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
		list<ActorRef*>& actorRefs();
};
//____________________________________________________________ Eventbus

template<typename Subscriber, typename Classifier> class SubscriberClassifier {
	public:
		Subscriber _subscriber;
		Classifier _classifier;
		SubscriberClassifier(Subscriber subscriber, Classifier classifier)
				: _subscriber(subscriber), _classifier(classifier) {
		}
};

//___________________________________________________________ Eventbus
template<typename ...> class EventBus;
template<typename Subscriber, typename Classifier, typename Event>
class EventBus<Event, Subscriber, Classifier> {
		list<SubscriberClassifier<Subscriber, Classifier>*> _list;

	public:
		void publish(Event& event) {
			Classifier cls = classify(event);
			for (SubscriberClassifier<Subscriber, Classifier>* sc : _list) {
				if (cls == sc->_classifier) push(event, sc->_subscriber);
			}
		}

		bool subscribe(Subscriber subscriber, Classifier classifier) {
			_list.push_back(new SubscriberClassifier<Subscriber, Classifier>(subscriber, classifier));
			return true;
		}

		bool unsubscribe(Subscriber, Classifier) {
			return false; // TODO not implemented yet
		}

		void unsubscribe(Subscriber) {
			return;
		}
		virtual Classifier classify(Event& event) = 0;
		virtual void push(Event& event, Subscriber subscriber) = 0;
		virtual int compareSubscribers(Subscriber a, Subscriber b) = 0;
		virtual ~EventBus() {
		}
};

class MessageClassifier {
		uid_type _src;
		uid_type _cls;
	public:

		MessageClassifier(Msg& msg) {
			_src = msg.src();
			_cls = msg.cls();
		}
		MessageClassifier(Uid uidSrc, Uid uidCls) {
			_src = uidSrc.id();
			_cls = uidCls.id();
		}

		bool operator==(MessageClassifier a) {
			return ((a._src == 0 || _src == 0 || a._src == _src)
					&& (a._cls == 0 || _cls == 0 || a._cls == _cls));
		}
};

class ActorMsgBus: public EventBus<Msg, ActorRef&, MessageClassifier> {
	public:
		void push(Msg& msg, ActorRef& ref) {
			msg.dst(ref.id());
//			INFO(" event : %s on mailbox : %X ",msg.toString().c_str(),&ref.mailbox());
			ref.mailbox().enqueue(msg);
		}
		MessageClassifier classify(Msg& msg) {
			return MessageClassifier(msg);
		}
		int compareSubscribers(ActorRef& a, ActorRef& b) {
			return a.id() == b.id();
		}
		~ActorMsgBus() {
		}
};

#endif /* SRC_AKKA_H_ */
