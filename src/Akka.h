/*
 * Akka.h
 *
 *  Created on: 2-aug.-2018
 *      Author: Lieven Merckx
 */

#ifndef SRC_AKKA_H_
#define SRC_AKKA_H_
#include <stdio.h>

/*============================================================================
 // Name        : akkaMicro.cpp
 // Author      : Lieven
 // Version     :
 // Copyright   : Enjoy teh source
 // Description : Akka alike framework in C++ for embedded systems : low RAM
 * Actor creation -> ActorCell
 * 				-> ActorRef
 * 				-> ActorContext -> empty Receive
 //
 // Mailbox 1 >-----> N Actor 1 <----> 1 ActorRef
 //
 // Actor -> ActorContext -> ActorSystem
 //                       -> Receive -> N Receiver ----> ( MsgClass,filter, method )
 *						 -> ActorRef
 //
 // ActorRef -> ActorCell -> Mailbox
 *  Creation sequence
 *  	ActorCell
 *  	ActorRef
 *  	ActorContext
 *  	Actor
 *
 *  	ActorRef contains id which is index to ActorCell and index to ActorContext

 Mailbox -> dispatches message based on ActorRef destination , ActorRef points to Mailbox  , points to Receive active
 //
 // 1 mailbox == 1 Thread
 // to avoid overhead of RTTI in embedded systems, a string identifier ( hashed )
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
//  ActorRef : shorthand uid_type reference
//

typedef void (*MsgHandler)(void);

class Envelope;
class Receiver;
class Mailbox;
class Dispatcher;
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

//_____________________________________________________________________ Message

typedef bool (*MsgMatch)(Envelope& msg);
typedef std::function<void(Envelope&)> MessageHandler;
typedef std::function<bool(Envelope&)> MessageMatcher;

extern ActorRef AnyActor;
extern MsgClass AnyClass;


extern Mailbox defaultMailbox;
extern Mailbox deadLetterMailbox;
extern ActorMsgBus bus;



class MsgClass {
	uid_type _id;
public:
	MsgClass(const char* name) :
			_id(Uid::hash(name)) {

	}
	const char* name() {
		return Uid::label(_id);
	}
	inline bool operator==(MsgClass m) {
		return m._id == _id;
	}
};

class Dispatcher {

};

class DispatcherFirstMatch : public Dispatcher {

};

class DispatcherAllMatch : public Dispatcher {

};

class Props {
	Mailbox& _mailbox;
	Dispatcher& _dispatcher;

public:
	Props& withMailbox(Mailbox& mailbox);
	template<class T> static Props create(const char* name, const char* fmt,
			...);
};
//_____________________________________________________________________ Receive
//
class Receive {
	LinkedList<Receiver*> receivers;

public:
	static Receive nullReceive;
	Receive();
	Receive& match(MsgClass msgClass, MessageHandler doSome);
	Receive& build();
	void handle(Envelope&);
};

//_____________________________________________________________________ Actor
//
class Actor {
public:
	virtual ~Actor();
	virtual ActorRef self()=0;
	virtual ActorRef sender()=0;
	virtual void preStart()=0;
	virtual void postStop()=0;
	virtual void unhandled(Envelope& msg)=0;

};
//_____________________________________________________________________ AbstractActor
//
class AbstractActor: public Actor {

	static LinkedList<AbstractActor*> actors;
	ActorContext* _context;

public:
	AbstractActor();
	virtual ~AbstractActor();

	void ask(ActorRef dst, Envelope& msg);

	void context(ActorContext* ctx);
	ActorContext& context();
	ActorSystem& system();
	void system(ActorSystem* sys);

	ActorRef self();
	ActorRef sender();

	virtual Receive& createReceive() = 0;
	Receive& receiveBuilder();
	virtual void preStart();
	virtual void postStop();
	virtual void unhandled(Envelope& msg);
//	virtual void handle(Envelope& msg){};
	Timer getTimers();
};
//__________________________________________________________ ActorRef
class ActorRef {
	uid_type _id;
public:
	static ActorRef noSender;
	static ActorRef anyActor;
	ActorRef();
	ActorRef(uid_type id);
	ActorRef(uid_type id, Mailbox*);

	uid_type id();
	void id(uid_type);

	bool operator==(ActorRef&);
//	void ask(ActorRef dst, MsgClass type, Envelope& msg, uint32_t timeout);
	void forward(Envelope& msg);
	void tell(ActorRef sender, Envelope& message);
	void tell(ActorRef sender, MsgClass type, const char* format, ...);
	Mailbox& mailbox();
	ActorRef& withMailbox();
	const char* path();
	void path(const char* p);
};

//________________________________________________________________ ActorCell

//_______________________________________________________________ ActorContext
class ActorContext {
	uid_type _id;
	ActorRef _self;
	const AbstractActor& _actor;
	const ActorSystem& _system;
	Mailbox& _mailbox;
	Receive& _receive;
	uint64_t _receiveTimeout;

	static LinkedList<ActorContext*> _actorContexts;

	void systemInvoke(Envelope& systemMessage);

public:

	ActorContext(uid_type id, ActorRef self, AbstractActor& actor,
			ActorSystem& system, Mailbox& mailbox, Receive& receive);
	uid_type id();
	static ActorContext& context(AbstractActor*);
	static ActorContext* context(ActorRef&);

	void become(Receive& receive);
	void unbecome();
	void cancelReceiveTimeout();
	void setReceiveTimeout(uint32_t msec);

	ActorSystem& system();
	ActorRef actorFor(const char* name);
	void system(ActorSystem&);
	Mailbox& mailbox();
	void mailbox(Mailbox&);
	void self(ActorRef&);
	ActorRef sender();
	ActorRef self();
	Receive& receive();
	void receive(Receive&);
	const char* path();
};

//_____________________________________________________________________
// ActorSystem
class ActorSystem {
	const char* _name;
	 Mailbox& _defaultMailbox;
	 Mailbox& _deadLetterMailbox;

public:

	ActorSystem(const char* name);
	uid_type uniqueId(const char* name); // check existing path and create new if needed #seq
//	Erc queue(Cbor& message);
//	void registerActor(AbstractActor&);
//	ActorRef addActor(AbstractActor& actor);
//	AbstractActor* findActor(ActorRef);
//	void deleteActor(ActorRef);
//	void callHandlers(ActorRef src, MsgClass type);
//	void loop();

//	void stop(ActorRef);

	ActorRef actorFor(const char* address);

	template<class T> ActorRef actorOf(const char* name, ...) {
		T* actor = new T();
		uid_type id = ActorSystem::uniqueId(name);
		ActorRef* actorRef=new ActorRef(id);
		ActorContext* context = new ActorContext(id, *actorRef, *actor,*this,_defaultMailbox,Receive::nullReceive);
		actor->context(context);
		context->receive(actor->createReceive());
		INFO(" new actor '%s' created",actorRef->path());
		return *actorRef;
	}
};

//_____________________________________________________________________ Message
class Envelope {
	static uid_type idCounter;

public:
	ActorRef sender;
	ActorRef receiver;
	MsgClass msgClass;
	uint32_t id;
	Cbor message;

	Envelope(uint32_t size);
	Envelope& setHeader(ActorRef snd, ActorRef rcv, MsgClass clz);
	bool getHeader();
	static uint32_t newId();
	bool scanf(const char* fmt, ...);
	void copyMessage(Envelope& dst);
	bool deserialize(Cbor& message);
	void serialize(Cbor& message);
	Str& toString(Str&);
};

//_____________________________________________________________________ Mailbox
class Mailbox {
	const char* _name;
	CborQueue _cborQueue;
	static LinkedList<Mailbox*> _mailboxes;

public:
	Envelope rxdEnvelope;
	Envelope txdEnvelope;
	Mailbox(const char* name,uint32_t queueSize, uint32_t messageSize);
	bool hasMessages();
	void enqueue(Envelope& msg);
	void dequeue(Envelope& msg);
	void handleMessage(Envelope& msg);
	void handleMessages();
};

extern Mailbox deadLetterMailbox;





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
	void handle(Envelope& msg);
	const static bool alwaysTrue(Envelope&) {
		return true;
	}
	Str& toString(Str& s);
};

//_____________________________________________________________________ Timer
//
class Timer {
	uid_type key;
	bool active;
	bool periodic;
	uint64_t expiresAt;

public:
	Timer(uid_type key, bool active, bool periodic, uint64_t interval);
};

class TimerScheduler {
	void startPeriodicTimer(uid_type key, Envelope& msg, uint32_t msec);
	void startSingleTimer(uid_type key, Envelope& msg, uint32_t msec);
	void cancel(uid_type key);
	void cancelAll();
	bool isTimerActive(uid_type key);
};

template<typename Subscriber, typename Classifier>
class SubscriberClassifier {
public:
	Subscriber _subscriber;
	Classifier _classifier;
	SubscriberClassifier(Subscriber subscriber, Classifier classifier) :
			_subscriber(subscriber), _classifier(classifier) {
	}
};

template<typename ...> class EventBus;
template<typename Subscriber, typename Classifier, typename Event>
class EventBus<Event, Subscriber, Classifier> {
	LinkedList<SubscriberClassifier<Subscriber, Classifier>*> _list;
public:
	void publish(Event event) {
		_list.forEach(
				[&event,this](SubscriberClassifier<Subscriber,Classifier>* sc) {
					if ( classify(event)==sc->_classifier) push(event,sc->_subscriber);
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
		return false; //TOD not implemented yet
	}
	void unsubscribe(Subscriber) {
		return;
	}
	virtual Classifier classify(Event event)=0;
	virtual void push(Event event, Subscriber subscriber)=0;
	virtual int compareSubscribers(Subscriber a, Subscriber b)=0;
	virtual ~EventBus() {
	}

};

class SenderMsgClass {
public:
	ActorRef _sender;
	MsgClass _msgClass;
	SenderMsgClass(ActorRef sender, MsgClass msgClass) :
			_sender(sender), _msgClass(msgClass) {
	}

	bool operator==(SenderMsgClass a) {
		return a._sender == _sender && a._msgClass == _msgClass;
	}
};

class ActorMsgBus: public EventBus<Envelope&, ActorRef, SenderMsgClass> {
public:

	void push(Envelope& envelope, ActorRef ref) {
		ref.mailbox().txdEnvelope.setHeader( envelope.sender, ref,envelope.msgClass);
		ref.mailbox().enqueue(ref.mailbox().txdEnvelope);
	}
	SenderMsgClass classify(Envelope& envelope) {
		return *(new SenderMsgClass(envelope.sender, envelope.msgClass));
	}
	int compareSubscribers(ActorRef a, ActorRef b) {
		return 1;
	}
	~ActorMsgBus() {

	}

};


#endif /* SRC_AKKA_H_ */
