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

extern ActorRef AnyActor;
extern ActorRef NoSender;
extern MsgClass AnyClass;
extern ActorSystem defaultActorSystem;
extern Receive NullReceive;
extern Mailbox defaultMailbox;
extern Mailbox deadLetterMailbox;
extern ActorMsgBus bus;

class MsgClass {
	uid_type _id;
public:
	MsgClass(const char* name) : _id(Uid::hash(name)){

	}
	const char* name(){
		return Uid::label(_id);
	}
	inline bool operator==(MsgClass m){
		return m._id==_id;
	}
};

class Props {
//	Mailbox& mailbox;

public:
	Props& withMailbox(Mailbox& mailbox);
	template<class T> static Props create(const char* name, const char* fmt,
			...);
};

//_____________________________________________________________________ Actor
class AbstractActor {

	static LinkedList<AbstractActor*> actors;

	ActorContext& _context;

public:
	AbstractActor();
	virtual ~AbstractActor();

	void ask(ActorRef dst, Envelope& msg);

	ActorContext& context();
	void context(ActorContext*);
	ActorSystem& system();
	void system(ActorSystem* sys);

	ActorRef self();
	ActorRef sender();

	virtual Receive& createReceive() = 0;
	Receive& receiveBuilder();
	virtual void preStart();
	virtual void postStop();
	virtual void unhandled(Envelope& msg);
	virtual void handle(Envelope& msg);
	Timer getTimers();
};
//__________________________________________________________ ActorRef
class ActorRef {
	uint16_t _id;
public:

	ActorRef();
	ActorRef(uid_type id);
	ActorRef(uid_type id, Mailbox*);

	uint16_t id();
	void id(uint16_t);

	bool operator==(ActorRef&);
	void ask(ActorRef dst, MsgClass type, Envelope& msg, uint32_t timeout);
	void forward(Envelope& msg);
	void tell(ActorRef sender, Envelope& message);
	void tell(ActorRef sender, MsgClass type, const char* format, ...);
	Mailbox& mailbox();
	ActorRef& withMailbox();
	const char* path();
	void path(const char* p);
};

//_____________________________________________________________________
// ActorContext
class ActorContext {
	uint16_t _idx;
	ActorRef& _self;
	ActorSystem& _system;
	Mailbox& _mailbox;
	Receive* _receive;
	uint64_t _timeout;

	static uint32_t _actorContextCounter;
	static ActorContext* _actorContexts[MAX_ACTOR_CELLS];

public:

	ActorContext();
	ActorContext(ActorRef&, ActorSystem&, Mailbox&, Receive*);

	static ActorContext& context(ActorRef&);

	void become(Receive& receive);
	void unbecome();
	void cancelReceiveTimeout();
	void setReceiveTimeout(uint32_t msec);

	ActorSystem& system();
	void system(ActorSystem&);
	Mailbox& mailbox();
	void mailbox(Mailbox&);
	void self(ActorRef&);
	ActorRef sender();
	ActorRef self();
	Receive& receive();
	void receive(Receive&);
};
//________________________________________________________________ ActorCell
class ActorCell {
	uint16_t _idx;	// auto initialized
	ActorRef& _ref; // auto initialized
	Mailbox& _mailbox; // set
	const char* _path;
	static uint32_t _actorCellCounter;
	static ActorCell* _actorCells[MAX_ACTOR_CELLS];
public:

	static ActorCell& cell(ActorRef ref);
	ActorCell();
	void mailbox(Mailbox&);

	ActorRef& ref();
	Mailbox& mailbox();

	const char* path();
	void path(const char* p);

};

//_____________________________________________________________________ Message
class Envelope {
	static uint16_t idCounter;

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
	bool deserialize(Cbor& message);
	void serialize(Cbor& message);
	Str& toString(Str&);
};

//_____________________________________________________________________ Mailbox
class Mailbox {
	CborQueue _cborQueue;
	static LinkedList<Mailbox*> _mailboxes;

public:
	Envelope rxdEnvelope;
	Envelope txdEnvelope;
	Mailbox(uint32_t queueSize, uint32_t messageSize);
	bool hasMessages();
	void enqueue(Envelope& msg);
	void dequeue(Envelope& msg);
	void handleMessage(Envelope& msg);
	void handleMessages();
};

extern Mailbox deadLetterMailbox;

//_____________________________________________________________________ Message

typedef bool (*MsgMatch)(Envelope& msg);
typedef std::function<void(Envelope&)> MessageHandler;
typedef std::function<bool(Envelope&)> MessageMatcher;

//_____________________________________________________________________
// ActorSystem
class ActorSystem {
	const char* _name;
	LinkedList<AbstractActor*> actors;

public:

	ActorSystem(const char* name);
	Erc queue(Cbor& message);
	void registerActor(AbstractActor&);
	ActorRef addActor(AbstractActor& actor);
	AbstractActor* findActor(ActorRef);
	void deleteActor(ActorRef);
	void callHandlers(ActorRef src, MsgClass type);
	void loop();

	void stop(ActorRef);
	template<class T> ActorRef actorOf(const char* name, ...) {
		T* actor = new T();
		actor->context().system(*this);
		actor->context().self().path(name);
		actor->createReceive();
		return actor->self();
	}
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
	void handle(Envelope& msg);
	const static bool alwaysTrue(Envelope&) {
		return true;
	}
	Str& toString(Str& s);
};
//_____________________________________________________________________ Receive
//
class Receive {
	LinkedList<Receiver*> receivers;
	uint32_t v;
	static uint32_t vCount;

public:
	Receive();
	Receive& match(MsgClass msgClass, MessageHandler doSome);
	Receive& build();
	void handle(Envelope&);
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
		ref.tell(NoSender, envelope);
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

class Thread {
};

class Dispatcher {
public:
	Mailbox& mailbox;
	Thread& thread;
	void dispatch() {
		// add message to mailbox
		// wakeup thread
	}
};

#endif /* SRC_AKKA_H_ */
