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


const uid_type UD_DST =H(AKKA_DST);
const uid_type UD_CLS =H(AKKA_CLS);
const uid_type UD_SRC= H(AKKA_SRC);
const uid_type UD_ID= H(AKKA_ID);
const uid_type UD_TIMER=H(AKKA_TIMER);





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
class TimerScheduler;

//_____________________________________________________________________ Static

//extern const MsgClass ReceiveTimeout;



extern ActorMsgBus eb;

//_____________________________________________________________________ Message

typedef bool (*MsgMatch)(Envelope& msg);
typedef std::function<void(Envelope&)> MessageHandler;
typedef std::function<bool(Envelope&)> MessageMatcher;

//______________________________JUST an IDEA _______________________________ Ref
//
/*class Reference {
		const char* _label;
		void *_pv;
		uid_type _cls;
		uid_type _id;
	public :
		Reference(const char* label,void *pv,uid_type cls,uid_type id) {
			_label=label;
			_pv=pv;
			_cls=cls;
			_id=id;
		}
		Reference(const char* label) {
			_label=label;
			_pv=0;
			_cls=0;
			_id=H(label);
		}
} ;

class Ref : public Uid {
		uid_type _id;
		static unordered_map<uid_type,Reference*>* _refs;
		static unordered_map<uid_type,Reference*>* refs() {
			if ( _refs==0) {
				_refs = new unordered_map<uid_type,Reference*>();
			}
		}
	public :
		Ref(const char* label,void* ths) {

		}
		template <typename T>  T* get(uid_type id) {
			std::unordered_map<uid_type, void*>::const_iterator p = refs()->find(id);
			if (p == uids()->end())
				return 0;
			return (T*)(p->second);
		}

};*/

//_____________________________________________________________ MsgClass
//
class MsgClass : public Uid {
	public:


		MsgClass() : Uid("NONE") {}
		MsgClass(uid_type id) : Uid(id) {}
		MsgClass(const char* name) : Uid(name) {}
		bool operator==(MsgClass b) { return id() == b.id(); };
};
//____________________________________________________________ Msg
//
class Msg : public Xdr {
		enum { OFF_CLS=1,OFF_SRC=3,OFF_DST=5,OFF_ID };
	public :
		Msg();
		~Msg();
		Msg(MsgClass cls) ;
		Msg(uint32_t);
		Msg(Uid cls,Uid src);
		Msg reply() ;
		Msg& clear();
		template <typename T> Msg& operator()(Uid key, T v) {
			add(key,v);
			return *this;
		};
		Msg& src(uid_type uid) ;
		Msg& dst(uid_type uid);
		Msg& cls(uid_type uid);
		Msg& id(uint32_t );
		uid_type src();
		uid_type dst();
		uid_type cls();
		uint32_t id();

		Msg& operator=(const Msg& );
};
//_________________________________________________________________ Timer
//
class Timer : public Uid {
		Msg* _msg;
		TimerHandle_t _timer;
		bool _autoReload;
		TimerScheduler& _timerScheduler;

	public:
		Timer(Uid key, bool autoReload, uint32_t interval,const Msg& msg,TimerScheduler& );
		~Timer();
		static void callBack(TimerHandle_t);
		void start();
		void stop();
		bool operator==(Timer&);
		Uid key();

		void interval(uint32_t);
		Msg& msg();
		void msg(const Msg&);
};
//__________________________________________________________ ActorRef
class ActorRef : public Uid {
		Mailbox* _mailbox;
		ActorCell* _cell;

		static unordered_map<uid_type, ActorRef*>* _actorRefs;
		static unordered_map<uid_type, ActorRef*>* actorRefs() {
			if ( _actorRefs==0 ) {
				_actorRefs = new unordered_map<uid_type, ActorRef*>();
			}
			return _actorRefs;
		}

	public:

		static ActorRef NoSender() ;
		ActorRef();
		ActorRef(Uid id);
		ActorRef(Uid id, Mailbox* mb);

		void ask(ActorRef dst, MsgClass type, Envelope& msg, uint32_t timeout);
		void forward(Envelope& msg);
		void tell(Envelope& message);
		void tell(Msg& msg);
		void tell(Msg& msg,ActorRef sender);
//		void tell(ActorRef& sender, MsgClass type, uint16_t id, const char* format,
//		...);
		void forward(Envelope& message, ActorContext& context);
		Mailbox& mailbox();
		void mailbox(Mailbox& mailbox);
		bool operator==(ActorRef src);
		const char* path();
		static ActorRef* lookup(Uid id);
		static uint32_t count() { return actorRefs()->size(); };
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
		Uid startPeriodicTimer(Uid key,const Msg&, uint32_t msec);
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
		Receive& receive(ActorCell&,Envelope&);
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
		virtual MessageDispatcher& dispatcher() = 0;

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
		Receive* _prevReceive;
		Actor* _actor;
		Envelope* _currentMessage;

		uint32_t _inactivityPeriod;
		uint64_t _lastReceive;
		bool _enable;
		TimerScheduler* _timers;

		static list<ActorCell*> _actorCells;

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
		void unhandled(Envelope& envelope);

		ActorRef& sender();

		void actor(Actor* actor);
		Actor* actor();
		static ActorCell* lookup(ActorRef* ref);
		static uint32_t count() { return _actorCells.size(); }

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
		static  MsgClass ReceiveTimeout();
//		static  MsgClass TimerExpired();
		static  MsgClass PoisonPill();
		static  MsgClass AnyClass();
		static  MsgClass Properties();
		static  MsgClass PropertiesReply();
		static 	void timerCallback(ActorRef&, Timer& );
		Actor();
		~Actor();
		ActorRef& self();
		ActorRef& sender();
		ActorContext& context();
		void context(ActorCell* context) { _context = context; }
		virtual void preStart() {};
		void aroundPrestart() { WARN(""); assert(false);};
		void postStop() { WARN(""); assert(false);};
		void unhandled(Envelope& msg);
		virtual Receive& createReceive() = 0;
		TimerScheduler& timers();
		Envelope& txdEnvelope();

		static Receive& receiveBuilder();
};

//________________________________________________________ ActorSelection

class ActorSelection : public ActorRef {
	public:
		ActorSelection(Uid id);
};

//________________________________________________________ Envelope
class Envelope : public Msg {
		static uint32_t _idCounter;

	public:
		ActorRef* sender;
		ActorRef* receiver;
		MsgClass msgClass;
		uint16_t id;
	public:
		Envelope();
		Envelope(ActorRef& snd, ActorRef& rcv, MsgClass clz);
		Envelope& operator=(Msg& );
		Envelope(ActorRef& snd, MsgClass clz);

		Envelope(ActorRef& snd, ActorRef& rcv, MsgClass clz, uint32_t size);
		Envelope& header(ActorRef& rcv, ActorRef& snd, MsgClass clz);
		Envelope& header(ActorRef& rcv, ActorRef& snd, MsgClass clz, uint16_t id);
		//    bool getHeader();
		static uint32_t newId();

//		void copyMessage(Envelope& dst);

};


//__________________________________________________________ MessageDispatcher
class MessageDispatcher {
		Mailbox* _mailbox;
//		list<Mailbox*> _mailboxes;
		list<ActorCell*> _actorCells;
		ActorCell* _unhandledCell;
		Envelope _txdEnvelope;
		Envelope _rxdEnvelope;
		uint64_t _nextWakeup;

	public:
		MessageDispatcher();
		void attach(Mailbox&);
		void detach(Mailbox&);
		void attach(ActorCell&);
		void detach(ActorCell&);
		void execute(bool loop=true);
		void resume(ActorCell&);
		void suspend(ActorCell&);
		void handle(Envelope&);
		void unhandled(ActorCell*);
		void nextWakeup(uint64_t t);
		uint64_t nextWakeup();
		Envelope& txdEnvelope();
};


//__________________________________________________________ Mailbox
class Mailbox  {
		QueueHandle_t _queue;
		const char* _name;
		Msg _txd;
//		Msg _rxd;
		SemaphoreHandle_t _sema;
//		static list<Mailbox*> _mailboxes;

	public:
		Mailbox(const char* name, uint32_t queueSize);
		int dequeue(Envelope&,uint32_t );
		int enqueue(Envelope&);
		int enqueue(Msg& );
		const char* name() { return _name;};
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
		bool match(Envelope& msg);
		void onMessage(Envelope& msg);
		const static bool alwaysTrue(Envelope&) { return true; }
		string& tostringing(string& s);
};

//_____________________________________________________________________ Props
//
class Props {
		MessageDispatcher* _dispatcher;
		Mailbox* _mailbox;

	public:
		Props(MessageDispatcher& d, Mailbox& mb) : _dispatcher(&d), _mailbox(&mb) {}
		Props() : _dispatcher(0), _mailbox(0) {}
		static Props& create() { return *new Props(); };
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
class ActorSystem : public Uid {
		const char* _name;
		Props _defaultProps;
		list<ActorRef*> _actorRefs;

	public:
		ActorSystem(const char* name, MessageDispatcher& defaultDispatcher,
		            Mailbox& defaultMailbox);
		Uid uniqueId(const char* name);
		ActorRef& actorFor(const char* address);

		template <class T> ActorRef& actorOf(const char* name, ...) {
			va_list args;
			va_start(args, name);
			T* actor = new T(args);
			va_end(args);

			return *create(actor, name, _defaultProps);
		}

		template <class T> ActorRef& actorOf(Props& props, const char* name, ...) {
			va_list args;
			va_start(args, name);
			T* actor = new T(args);
			va_end(args);

			return *create(actor, name, props);
		}

		ActorRef* create(Actor* actor, const char* name, Props& props) ;
		MessageDispatcher& defaultDispatcher() { return _defaultProps.dispatcher(); }
		Mailbox& defaultMailbox() { return _defaultProps.mailbox(); }
		list<ActorRef*>& actorRefs() { return _actorRefs;};
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
		list<SubscriberClassifier<Subscriber, Classifier>*> _list;

	public:
		void publish(Event& event) {
			Classifier cls = classify(event) ;
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
};

class MessageClassifier {
		uid_type _src;
		uid_type _cls;
	public:

		MessageClassifier(Msg& msg) {
			_src = msg.src();
			_cls = msg.cls();
		}
		MessageClassifier(Uid uidSrc,Uid uidCls) {
			_src = uidSrc.id();
			_cls = uidCls.id();
		}

		bool operator==(MessageClassifier a) {
			return ( (a._src==0 || _src==0 || a._src==_src )&&
			         (a._cls==0 || _cls==0 || a._cls==_cls ));
		}
};

class ActorMsgBus : public EventBus<Msg, ActorRef&, MessageClassifier> {
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
		~ActorMsgBus() {}
};

class Thread {
		uint32_t _signal;

	public:
		void signal(uint32_t signal);
		uint32_t waitSignal(uint32_t msec);
};

#endif /* SRC_AKKA_H_ */
