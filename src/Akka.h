/*
 * Akka.h
 *
 *  Created on: 2-aug.-2018
 *      Author: D-MG61DD
 */

#ifndef SRC_AKKA_H_
#define SRC_AKKA_H_
#include <stdio.h>

//============================================================================
// Name        : akkaMicro.cpp
// Author      : Lieven
// Version     :
// Copyright   : Enjoy teh source
// Description : Akka alike framework in C++ for embedded systems : low RAM
//============================================================================

#include <iostream>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <functional>

using namespace std;

#include <Erc.h>
#include <Cbor.h>
#include <CborQueue.h>
#include <Str.h>
#include <LinkedList.hpp>
#include <Log.h>
#include <Uid.h>

//#define MAX_FMT_LEN 22

// ( Actor -> mailbox -> thread )
// all coRoutines == same mailbox, a single thread
//  ActorRef : shorthand uid_t reference
//

typedef uid_t MsgClass;
typedef void (*MsgHandler)(void);

class MessageQueue: public CborQueue
{
public:
	MessageQueue(uint32_t size);
};

class MailBox: public MessageQueue
{
public:
	MailBox(uint32_t size);
	bool hasMessages();
	void awake();
	void run();
};

class Message;

class ActorRef
{

public:
	uid_t uid;
	ActorRef();
	ActorRef(uid_type id);
	void ask(ActorRef dst, MsgClass type, Message& msg, uint32_t timeout);
	void forward(Message& msg);
	void tell(Message& message, ActorRef sender);
	void tell(ActorRef dst, MsgClass type, const char* format, ...) const;

};

//_____________________________________________________________________ Envelope
class Message
{
	static uint32_t idCounter;
public:
	ActorRef sender;
	ActorRef receiver;
	MsgClass msgClass;
	uint32_t id;
	Cbor payload;
	Message(uint32_t size);
	void setHeader(ActorRef snd, ActorRef rcv, MsgClass clz);
	static uint32_t newId();
	bool read(const char* fmt,...);
	bool deserialize(Cbor& message);
	void serialize(Cbor& message);
};

//_____________________________________________________________________ Mailbox
class Mailbox {
	CborQueue cborQueue;
public :
	Mailbox(uint32_t size);
	bool hasMessages();
	void enqueue(Message& msg);
	void dequeue(Message& msg);
};

//_____________________________________________________________________ Message


typedef bool (*MsgMatch)(Message& msg);
typedef std::function<void(Message&)> MessageHandler;

class Actor;
//_____________________________________________________________________ ActorSystem
class ActorSystem
{
	const char* _name;


public:
	Mailbox mailbox;
	ActorSystem(const char* name, uint32_t queueSize);
	Erc queue(Cbor& message);
	void registerActor(Actor&);
	void addActor(Actor& actor);
	Actor* findActor(ActorRef);
	void deleteActor(ActorRef);
	void callHandlers(ActorRef src, MsgClass type);
	void loop();
	void setReceiveTimeout(uint32_t msec);
	uint32_t getReceiveTimeout();
	void stop(ActorRef);
	template<class T> ActorRef actorOf(const char* name, ...)
	{
		T* t = new T(name);
		addActor(t);
		return t->self();
	}
};

class Context: public ActorSystem
{

};

//_____________________________________________________________________ Receive
class Receive
{
	// table of src,msgType,bool func, function(Message)
public:
	Receive& match(ActorRef dst, MsgClass msgClass, MessageHandler f);
	Receive& build();
};
//_____________________________________________________________________ Actor
class Actor
{

	// Linkedlist<MsgHandler> first;
public:
	const char* name;
	const ActorSystem& system;
	const ActorRef self;

	Actor(ActorSystem& system, const char* name);
	virtual ~Actor();
	ActorRef getDestination();
	Erc getMessage(const char* fmt, ...);

	void event(MsgClass type, const char* fmt, ...); // queue message src,ANY_ACTOR,msgType,idx,...
	void ask(ActorRef dst, Message& msg);

	ActorRef getSelf();
	ActorRef getSender();
	const char* getName();
	virtual Receive& createReceive() =0;
	ActorSystem& getSystem();
	Context& getContext();
	Receive& receiveBuilder();
	virtual void preStart();
	virtual void postStop();
	virtual void unhandled(Message& msg);
};

class Dispatcher
{
public :

};

extern ActorRef AnyActor;
extern uid_type AnyClass;
extern ActorSystem actorSystem;

#endif /* SRC_AKKA_H_ */
