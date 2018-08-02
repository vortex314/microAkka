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

//#define MAX_FMT_LEN 22

// ( Actor -> mailbox -> thread )
// all coRoutines == same mailbox, a single thread
//  ActorRef : shorthand uid_t reference
//

typedef uid_t MsgClass;
typedef void (*MsgHandler)(void);

MessageQueue::MessageQueue(uint32_t size) :
		CborQueue(size) {
}

ActorRef::ActorRef(uid_type id) {
	uid = id;
}
void ActorRef::ask(ActorRef dst, MsgClass type, Message& msg,
		uint32_t timeout) {

}
void ActorRef::forward(Message& msg) {

}

void ActorRef::tell(Message& message, ActorRef sender) {

}

void ActorRef::tell(ActorRef dst, MsgClass type, const char* format,
		...) const { // queue message src,dst,msgtype,idx,...
	Message msg(*this, dst, type, 1024);

}

ActorRef AnyActor(0);
uid_type AnyClass = 0;
//_____________________________________________________________________ Envelope
uint32_t Envelope::idCounter = 0;
Envelope::Envelope() {

}
Envelope::Envelope(ActorRef snd, ActorRef rcv, MsgClass clz) :
		sender(snd), receiver(rcv), msgClass(clz), id(idCounter++) {

}
bool Envelope::read(Cbor& message) {
	return message.scanf("iiii", &sender.uid, &receiver.uid, &msgClass, &id);
}

void Envelope::write(Cbor& message) {
	message.addf("iiii", sender.uid, receiver.uid, msgClass, id);
}

bool Message::read(Cbor& message, const char* fmt, ...) {
	payload = message;
	if (Envelope::read(message)) {
		va_list args;
		va_start(args, fmt);
		bool b = payload.vscanf(fmt, args);
		va_end(args);
		return b;
	}
	return false;
}

Message::Message(ActorRef snd, ActorRef rcv, MsgClass clz, uint32_t size) :
		Envelope(snd, rcv, clz), payload(size) {
}

Message::Message(Cbor& in) :
		payload(in) {
	readEnvelope();
}

void Message::writeEnvelope() {
	Envelope::write (_message);
}
bool Message::readEnvelope() {
	return Envelope::read(payload);
}

typedef bool (*MsgMatch)(Message& msg);
typedef std::function<void(Message&)> MessageHandler;

//_____________________________________________________________________ ActorSystem

ActorSystem::ActorSystem(const char* name, uint32_t queueSize) :
		_name(name), _queue(queueSize) {

}

void ActorSystem::addActor(Actor& actor) {

}

void ActorSystem::setReceiveTimeout(uint32_t msec) {
}
uint32_t ActorSystem::getReceiveTimeout() {
	return 0;
}
void ActorSystem::stop(ActorRef) {
}

//_____________________________________________________________________ Receive

Receive& Receive::match(ActorRef dst, MsgClass msgClass, MessageHandler f) {
}
Receive& Receive::build() {
}

//_____________________________________________________________________ Actor

Actor::Actor(ActorSystem& system, const char* name) :
		name(name), system(system), self(H(name)) {
	system.addActor(*this);
}
Actor::~Actor() {
}
Receive& Actor::receiveBuilder() {
	return *new Receive();
}
void Actor::preStart() {
}
void Actor::postStop() {
}
void Actor::unhandled(Message& msg) {
	INFO("unhandled message for Actor : %s ", name);
}

