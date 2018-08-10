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
#include <iostream>
#include <stdarg.h>
#include <stdint.h>
#include <string.h>

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

typedef uid_type MsgClass;
typedef void (*MsgHandler)(void);

class Envelope;
class Receiver;
class Mailbox;
class Actor;
class ActorContext;
class ActorRef;
class ActorSystem;
class ActorCell;
class Envelope;
class Message;
class SystemMessage;
class Receive;
class Props;


extern ActorRef anyActor;
extern ActorRef noSender;
extern uid_type AnyClass;
extern ActorSystem defaultActorSystem;
extern Receive nullReceive;
extern Mailbox defaultMailbox;
extern Mailbox deadLetterMailbox;

class Props {
//	Mailbox& mailbox;

public:
	Props& withMailbox(Mailbox& mailbox);
	template <class T>  static Props create(const char* name,const char* fmt,...);
};

class ActorRef {
	uint16_t _id;
public:

	ActorRef();
	ActorRef(uid_type id);
	ActorRef(uid_type id, Mailbox*);

	uint16_t id();

	bool operator==(ActorRef&);
	void ask(ActorRef dst, MsgClass type, Envelope& msg, uint32_t timeout);
	void forward(Envelope& msg);
	void tell(Envelope& message, ActorRef sender);
	void tell(ActorRef sender, MsgClass type, const char* format, ...);
	void operator<<(Envelope& message);
	Mailbox& mailbox();
	ActorRef& withMailbox();
	const char* path();
};

//_____________________________________________________________________
// ActorContext
class ActorContext {
	ActorRef& _self;
	ActorSystem& _system;
	Mailbox& _mailbox;
	Receive& _receive;
	uint64_t _timeout;

	static uint32_t _actorContextCounter;
	static ActorContext* _actorContexts[MAX_ACTOR_CELLS];

public:

	ActorContext();
	ActorContext(ActorRef&,ActorSystem&,Mailbox&,Receive&);

	void become(Receive& receive);
	void unbecome();
	void cancelReceiveTimeout();
	void setReceiveTimeout(uint32_t msec);

	ActorSystem& system();
	void system(ActorSystem&);
	Mailbox& mailbox();
	void mailbox(Mailbox&);
	void self(ActorSystem&);
	ActorRef sender();
	ActorRef self();
	Receive& receive();


};

class ActorCell {
	uint16_t _idx;	// auto initialized
	ActorRef& _ref; // auto initialized
	Mailbox& _mailbox; // set
	static uint32_t _actorCellCounter;
	static ActorCell* _actorCells[MAX_ACTOR_CELLS];
public:

	static ActorCell& cell(ActorRef ref);
	ActorCell();
	void mailbox(Mailbox&);

	ActorRef& ref();
	Mailbox& mailbox();

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
	LinkedList<Actor*> actors;

public:

	ActorSystem(const char* name);
	Erc queue(Cbor& message);
	void registerActor(Actor&);
	ActorRef addActor(Actor& actor);
	Actor* findActor(ActorRef);
	void deleteActor(ActorRef);
	void callHandlers(ActorRef src, MsgClass type);
	void loop();

	void stop(ActorRef);
	template<class T> ActorRef actorOf(const char* name, ...) {
		T* actor = new T(name);
		actor->context().system(*this);
		addActor(*actor);
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
//_____________________________________________________________________ Actor
class Actor {

	static LinkedList<Actor*> actors;
	const char* _name;
	ActorContext& _context;

public:



	Actor(const char* name);
	virtual ~Actor();
	ActorRef getDestination();

	void event(MsgClass type, const char* fmt, ...); // queue message src,ANY_ACTOR,msgType,idx,...
	void ask(ActorRef dst, Envelope& msg);
	// 	AbstractActor
	//------------------
	void setReceiveTimeout(uint32_t msec);
	uint32_t getReceiveTimeout();
	ActorContext& context();
	void context(ActorContext*);
	ActorSystem& system();
	void system(ActorSystem* sys);

	ActorRef self();
	ActorRef sender();
	const char* getName();
	virtual Receive& createReceive() = 0;
	Receive& receiveBuilder();
	virtual void preStart();
	virtual void postStop();
	virtual void unhandled(Envelope& msg);
	Timer getTimers();
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
