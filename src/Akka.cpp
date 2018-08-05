/*
 * Akka.cpp
 *
 *  Created on: 2-aug.-2018
 *      Author: D-MG61DD
 */

#include <Akka.h>
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

//_____________________________________________- STATIC
ActorRef AnyActor(0);
uid_type AnyClass = 0;
ActorSystem actorSystem("system",2000,1024);
//_______________________________________________

//#define MAX_FMT_LEN 22

// ( Actor -> mailbox -> thread )
// all coRoutines == same mailbox, a single thread
//  ActorRef : shorthand uid_t reference
//

typedef uid_t MsgClass;
typedef void (*MsgHandler)(void);


ActorRef::ActorRef(){
	
}

ActorRef::ActorRef(uid_type id)
{
	uid = id;
}
void ActorRef::ask(ActorRef dst, MsgClass type, Message& msg,
                   uint32_t timeout)
{

}
void ActorRef::forward(Message& msg)
{

}

void ActorRef::tell(Message& message, ActorRef sender)
{

}

void ActorRef::tell(ActorRef src, MsgClass cls, const char* fmt,...) const
{
	Message msg(1024);

	msg.payload.addf("iiii",src.uid,uid,cls,Message::newId());
	va_list args;
	va_start(args, fmt);
	msg.payload.vaddf(fmt,args);
	va_end(args);
	actorSystem.mailbox.enqueue(msg);
}



uint32_t Message::idCounter = 0;
uint32_t Message::newId()
{
	return idCounter++;
}

//_____________________________________________________________________________ Message
//


Message::Message(uint32_t size) :  payload(size)
{
}



bool Message::read(const char* fmt, ...)
{
	payload.offset(0);
	if (payload.scanf("iiii",&sender.uid,&receiver.uid,&msgClass,&id)) {
		va_list args;
		va_start(args, fmt);
		bool b = payload.vscanf(fmt, args);
		va_end(args);
		return b;
	}
	return false;
}

typedef bool (*MsgMatch)(Message& msg);
typedef std::function<void(Message&)> MessageHandler;
//_____________________________________________________________________ Mailbox
Mailbox::Mailbox(uint32_t queueSize,uint32_t messageSize) : cborQueue(queueSize),rxdMessage(messageSize),txdMessage(messageSize) {
	
}

void Mailbox::enqueue(Message& msg) {
	cborQueue.put(msg.payload);
}
void Mailbox::dequeue(Message& msg) {
	cborQueue.get(msg.payload);
}

bool Mailbox::hasMessages(){
	return cborQueue.hasData();
}
//_____________________________________________________________________ ActorSystem
//
ActorSystem::ActorSystem(const char* name, uint32_t queueSize,uint32_t messageSize) :
	_name(name), mailbox(queueSize,messageSize)
{

}

void ActorSystem::addActor(Actor& actor)
{

}

void ActorSystem::setReceiveTimeout(uint32_t msec)
{
}
uint32_t ActorSystem::getReceiveTimeout()
{
	return 0;
}
void ActorSystem::stop(ActorRef)
{
}

//_____________________________________________________________________ Receive

Receive& Receive::match(ActorRef dst, MsgClass msgClass, MessageHandler f)
{
	return *this;
}
Receive& Receive::build()
{
	return *this;
}

//_____________________________________________________________________ ActorContext
//
ActorContext::ActorContext(){
	
}

//_____________________________________________________________________ Actor

Actor::Actor(ActorSystem& system, const char* name) :
	name(name), system(system), self(H(name))
{
	system.addActor(*this);
}
Actor::~Actor()
{
}
Receive& Actor::receiveBuilder()
{
	return *new Receive();
}
void Actor::preStart()
{
}
void Actor::postStop()
{
}
void Actor::unhandled(Message& msg)
{
	INFO("unhandled message for Actor : %s ", name);
}
