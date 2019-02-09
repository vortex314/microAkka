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
#include <atomic>
//using namespace std;
// Common
//#include <Label.h>
#include <Xdr.h>

#define myASSERT(xxx) if (!(xxx) ) {WARN(" assertion " # xxx " failed.");};

// FreeRTOS

#include <FreeRTOS.h>
#include <semphr.h>
#include <queue.h>
#include <task.h>
#include <timers.h>


class NativeQueue {
		QueueHandle_t _queue;
	public:
		NativeQueue(uint32_t queueSize,uint32_t itemSize) {
			_queue = xQueueCreate(queueSize, itemSize);
		}
		int send(void* item,uint32_t msecWait){
			BaseType_t rc = xQueueSend(_queue, item, pdMS_TO_TICKS(msecWait) );
			if ( rc == pdTRUE ) return 0;
			return rc;
		}
		int recv(void* item,uint32_t msecWait) {
			if (xQueueReceive(_queue, item, pdMS_TO_TICKS(msecWait)) != pdTRUE) {
					return ENOENT;
				}
			return 0;
		}
		bool hasMessages(){
			return uxQueueMessagesWaiting(_queue);
		}
};

typedef void (*TimerCallback)(void*);


class NativeTimer {
		TimerHandle_t _timer;
		TimerCallback _callbackFunction;
		void* _callbackArg;
		bool _autoReload;
		uint32_t _interval;
	public:
		static void freeRTOSCallback(TimerHandle_t handle){
			NativeTimer* me = (NativeTimer*) pvTimerGetTimerID(handle);
			me->_callbackFunction(me->_callbackArg);
		}

		NativeTimer(const char* name,bool autoReload , uint32_t interval,void* callbackArg,TimerCallback callbackFunction) :
			_callbackFunction(callbackFunction),
			_callbackArg(callbackArg),
			_autoReload(autoReload),
			_interval(interval) {
			configASSERT((_timer = xTimerCreate(name,pdMS_TO_TICKS(interval),autoReload,this,freeRTOSCallback))!=NULL);
		}

		~NativeTimer(){
		}

		void start(){
			configASSERT(xTimerStart(_timer,0) == pdPASS);
		}

		void stop() {
			configASSERT(xTimerStop(_timer,0)==pdPASS);
		}

		void reset() {
			configASSERT(xTimerReset(_timer,0)==pdPASS);
		}

		void interval(uint32_t v){
			INFO("[%X] timer interval(%u)", this, v);
			configASSERT(xTimerChangePeriod(_timer,pdMS_TO_TICKS(v),10)==pdPASS);
		}


};

typedef void(*TaskFunction)(void*);

class NativeThread {
		const char* _name;
		uint32_t _stackSize = 1024;
		uint32_t _priority = tskIDLE_PRIORITY + 1;
		TaskFunction _taskFunction;
		void* _taskArg;
		TaskHandle_t _task;
	public:
		 NativeThread(const char* name,uint32_t stackSize,uint32_t priority,TaskFunction taskFunction){
			 _name=name;
			_stackSize=stackSize;
			_priority=priority;
			_taskFunction = taskFunction;
			_task=0;
		}
		void start(){
			BaseType_t rc = xTaskCreate(_taskFunction,_name, _stackSize, _taskArg,_priority, &_task);
				myASSERT(rc==pdPASS);
		}
		void wait();
};



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
 * Label  => ActorRef => ActorCell => ActorContext => AbstractActor
 *                                  => Mailbox
 * Label  => Abs
 * Label  => MsgClass
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

#define LBL(__str__) Ref(UID(__str__),__str__)

class Label {
		typedef struct {
				uid_type _uid;
				const char* _label;
		} LabelStruct;
		LabelStruct* _pl = 0; // single element to use in varargs, on stack
		static std::unordered_map<uid_type, LabelStruct*>* _labels;

	public:
		Label(uid_type uid, const char* label) {
			if (labels()->find(uid) == labels()->end()) {
				_pl = new LabelStruct;
				_pl->_label = new char[strlen(label) + 1];
				strcpy((char*) _pl->_label, label);
				_pl->_uid = uid;
				labels()->emplace(uid, _pl);
			} else {
				_pl = labels()->find(uid)->second;
			}
		}
		Label(const char* label);
		Label(uid_type id);
		Label();
		static const char* label(uid_type);
		static std::unordered_map<uint16_t, LabelStruct*>* labels();
		uid_type id();
		const char * label();
		bool operator==(Label& that);
};

class Ref: public Label {
		typedef struct {
				Label _label;
				void* _object;
				Label _cls;
		} RefStruct;
		RefStruct* _pr;
		static std::unordered_map<uid_type, RefStruct*> _refs;
	public:
		Ref(Label label, void* object, Label cls) {
			if (_refs.find(label.id()) == _refs.end()) {
				_pr = new RefStruct( { label, object, cls });
				/*							_pr->_label=label;
				 _pr->_cls=cls;
				 _pr->_object=object;*/
				_refs.emplace(label.id(), _pr);
			}
		}
		Ref(RefStruct* pr)
				: _pr(pr) {
		}
		const char* label();
		uid_type id();
		Label cls();
		void *object();

		~Ref(); // the object destruction should lead to Ref lookup destruction
		static Ref findRef(uid_type);
		static Ref NotFound;
		bool operator==(Ref&);

};

//_____________________________________________________________ MsgClass
//
class MsgClass: public Label {
	public:
		static MsgClass ReceiveTimeout();
//		static  MsgClass TimerExpired();
		static MsgClass PoisonPill();
		static MsgClass AnyClass();
		static MsgClass Properties();
		static MsgClass PropertiesReply();
		MsgClass()
				: Label("NONE") {
		}
		MsgClass(uid_type id)
				: Label(id) {
		}
		MsgClass(const char* name)
				: Label(name) {
		}
		bool operator==(MsgClass b) {
			return id() == b.id();
		}
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
		Msg(Label cls, Label src);
		Msg& reply(Msg& req);
		Msg& clear();
		template<typename T> Msg& operator()(Label key, T v) {
			add((uid_type) key.id(), v);
			return *this;
		}
		;
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

		Msg& operator=(const Msg&);
		std::string toString();
};

typedef bool (*MessageMatch)(Msg& msg);
typedef std::function<void(Msg&)> MessageHandler;
typedef std::function<bool(Msg&)> MessageMatcher;

//_________________________________________________________________ Timer
//

class Timer: public Label,public NativeTimer {
		Msg* _msg;
		bool _autoReload;
		TimerScheduler& _timerScheduler;

	public:
		Timer(Label key, bool autoReload, uint32_t interval, const Msg& msg,
				TimerScheduler&);
		~Timer();
		static void callBack(void* );

		bool operator==(Timer&);
		Label key();
		Msg& msg();
		void msg(const Msg&);
};

//__________________________________________________________ ActorRef

class ActorRef: public Ref {

	public:
		static ActorRef& NoSender();
		static ActorRef* lookup(uid_type id);

		ActorRef(Label);
		bool operator==(ActorRef&);
		const char* path();

		virtual ~ActorRef();
		virtual void tell(Msg& msg)=0;
		virtual void tell(Msg& msg, ActorRef& sender)=0;
		virtual void forward(Msg& message, ActorContext& context)=0;
		virtual bool isLocal() = 0;

};

//______________________________________________________ LocalActorRef
//

class LocalActorRef: public ActorRef {
		ActorCell& _cell;

	public:

		static ActorRef& NoSender();
		LocalActorRef(Label, ActorSystem&, Props&, MessageDispatcher&);
		~LocalActorRef();

		Mailbox& mailbox();
		void mailbox(Mailbox& mailbox);

		void forward(Msg& msg);
		void tell(Msg& msg);
		void tell(Msg& msg, ActorRef& sender);
		void forward(Msg& message, ActorContext& context);
		bool operator==(ActorRef& src);
		bool isLocal() {
			return true;
		}
		void cell(ActorCell& cell);
		ActorCell& cell();

};

//____________________________________________________ RemoteActorRef
//

class RemoteActorRef: public ActorRef {
		ActorRef& _bridge;

	public:
		RemoteActorRef(Label, ActorRef&);
		void tell(Msg& msg) {
			_bridge.tell(msg);
		}
		void tell(Msg& msg, ActorRef& sender){
			_bridge.tell(msg,sender);
		}
		void forward(Msg& message, ActorContext& context) {};
		bool operator==(ActorRef& src){ return id()==src.id();}
		bool isLocal() {
			return false;
		}
};

//___________________________________________________ EmptyLocalActorRef
class EmptyLocalActorRef {
	public:

};

//__________________________________________________________ TimerScheduler
//

class TimerScheduler {
		std::list<Timer*> _timers;
		ActorRef& _ref;

	public:
		void timerCallback(Timer&);
		TimerScheduler(ActorRef&);
		Timer* find(Label key);
		Timer* findNextTimeout();
		Label startPeriodicTimer(Label key, const Msg&, uint32_t msec);
		Label startSingleTimer(Label key, const Msg&, uint32_t msec);
		void cancel(Label key);
		void cancelAll();
		bool isTimerActive(Label key);
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
class ActorContext: public ActorRefFactory {

		void systemInvoke(Msg& systemMessage);

	public:
		virtual ActorRef& self() = 0;
		virtual uint32_t receiveTimeout() = 0;
		virtual void setReceiveTimeout(uint32_t msec) = 0;
		void become(Receive& receive) {
			become(receive, true);
		}
		virtual void become(Receive& receive, bool discardOld) = 0;
		virtual void unbecome() = 0;
		virtual ActorRef& sender() = 0;
		virtual ActorSystem& system() = 0;
//		virtual MessageDispatcher& dispatcher() = 0;
		virtual Thread& currentThread()=0;

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
class ActorCell: public ActorContext {
		Mailbox& _mailbox;
		MessageDispatcher& _dispatcher;
		ActorSystem& _system;
		ActorRef& _self;
		Actor*_actor;

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
		~ActorCell();

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
		static uint32_t count() {
			return _actorCells.size();
		}

		const char* path();
		void resetReceiveTimeout();
		TimerScheduler& timers();
		bool hasTimers();

		bool hasReceiveTimedOut();
		uint64_t expiresAt();
		static std::list<ActorCell*>& actorCells();

		void sendMessage(Msg& msg);
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
		void context(ActorCell* context) {
			_context = context;
		}
		virtual void preStart() {
		}
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

//________________________________________________________ ActorSelection

class ActorSelection: public ActorRef {
	public:
		ActorSelection(Label id);
};

//__________________________________________________________ Thread
//
// per thread data
//
class Thread: public Ref,public NativeThread {
		Msg _txd;
		Msg _rxd;
		MessageDispatcher* _dispatcher;
	public:
		Thread(MessageDispatcher* dispatcher, const char* name,
				uint32_t stackSize ,
				uint32_t priority ,void* threadArg,TaskFunction f);
		void currentMessage(Msg* msg);
		Msg& currentMessage();
		Msg& txd();
		Msg& rxd();
		MessageDispatcher& dispatcher();
};

//__________________________________________________________ MessageDispatcher
class MessageDispatcher  {
		std::list<ActorCell*> _actorCells;
		ActorCell* _unhandledCell;

		uint32_t _threadCount = 1;
		uint32_t _throughput = 10;
		std::list<Thread*> _threads;
		NativeQueue _workQueue;
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

		void dispatch(ActorCell&, Msg&);
		void registerForExecution(Mailbox*);
		NativeQueue& workQueue();

};

//__________________________________________________________ Mailbox
class Mailbox : public NativeQueue {
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
		Receiver(MsgClass msgClass, MessageMatcher matcher,
				MessageHandler handler);
		Receiver(MsgClass msgClass, MessageHandler handler);
		bool match(Msg& msg);
		void onMessage(Msg& msg);
		static bool alwaysTrue(Msg&) {
			return true;
		}
		std::string& toString(std::string& s);
};

//_____________________________________________________________________ Props
//
class Props {
		MessageDispatcher* _dispatcher;

	public:
		Props(MessageDispatcher& d)
				: _dispatcher(&d) {
		}
		Props()
				: _dispatcher(0) {
		}
		static Props& create() {
			return *new Props();
		}
		;
		Props& withDispatcher(MessageDispatcher& dispatcher) {
			_dispatcher = &dispatcher;
			return *this;
		}

		MessageDispatcher& dispatcher() {
			return *_dispatcher;
		}

};
//___________________________________________________________ ActorSystem
class ActorSystem: public Ref {
		Props _defaultProps;
		std::list<ActorRef*> _actorRefs;
		MessageDispatcher _defaultDispatcher;

	public:
		ActorSystem(Label, MessageDispatcher& defaultDispatcher);
		Label uniqueId(const char* name);
		ActorRef& actorFor(const char* address);

		template<class T, typename ... Args> ActorRef& actorOf(const char* name,
				Args&& ... args) {
			T* actor = new T(args...);
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
		std::list<ActorRef*>& actorRefs();
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
		std::list<SubscriberClassifier<Subscriber, Classifier>*> _list;

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
		MessageClassifier(ActorRef& uidSrc, Label uidCls) {
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
			ref.tell(msg);
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
