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
// Copyright   : Enjoy the source
// Description : Akka alike framework in C++ for embedded systems : low RAM
//============================================================================

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

//_____________________________________________- STATIC
ActorRef AnyActor(0);
ActorRef NoSender(1);
uid_type AnyClass = 0;
Mailbox deadLetter(1, 10);

typedef uid_type MsgClass;
typedef void (*MsgHandler)(void);

ActorRef::ActorRef() :
		id(NoSender.id) {
}
ActorRef::ActorRef(uid_type id) :
		id(id) {
}

bool ActorRef::operator==(ActorRef& dst) {
	return (id == dst.id);
}

void ActorRef::ask(ActorRef dst, MsgClass type, Envelope& msg,
		uint32_t timeout) {
}

void ActorRef::tell(ActorRef src, MsgClass cls, const char* fmt, ...) {

	Mailbox& srcMailbox = src.mailbox();
	Mailbox& dstMailbox = mailbox();
	va_list args;
	va_start(args, fmt);
	srcMailbox.txdEnvelope.setHeader(src, id, cls).message.vaddf(fmt, args);
	va_end(args);

	dstMailbox.enqueue(srcMailbox.txdEnvelope);
}

void ActorRef::forward(Envelope& msg) {
	Mailbox& rcvMailbox = mailbox();
	Mailbox& sndMailbox = msg.receiver.mailbox();
	sndMailbox.txdEnvelope.setHeader(msg.sender, id, msg.msgClass);
	sndMailbox.txdEnvelope.message.append(msg.message);
	rcvMailbox.enqueue(sndMailbox.txdEnvelope);
}

uint16_t Envelope::idCounter = 0;

uint32_t Envelope::newId() {
	return idCounter++;
}

//______________________________________________________________________ Message
//

Envelope::Envelope(uint32_t size) :
		 sender(AnyActor), receiver(AnyActor), msgClass(
				AnyClass),id(0),message(size) {
}

Envelope& Envelope::setHeader(ActorRef snd, ActorRef rcv, MsgClass clz) {
	message.offset(0);
	sender = snd;
	receiver = rcv;
	msgClass = clz;
	id = newId();
	message.addf("2222", snd.id, rcv.id, clz, id);
	return *this;
}

bool Envelope::getHeader() {
	return message.scanf("2222", &sender.id, &receiver.id, &msgClass, &id);
}

bool Envelope::scanf(const char* fmt, ...) {
	bool b = false;
	va_list args;
	va_start(args, fmt);
	b = message.vscanf(fmt, args);
	va_end(args);
	return b;
}

Str& Envelope::toString(Str& s) {
	return s.format(" dst : %s , src : %s , class : %s  ",
			receiver.path(),sender.path(), UID.label(msgClass));
}

typedef bool (*MsgMatch)(Envelope& msg);

typedef std::function<void(Envelope&)> MessageHandler;

//_____________________________________________________________________ Mailbox

LinkedList<Mailbox*> mailboxes();

Mailbox::Mailbox(uint32_t queueSize, uint32_t messageSize) :
		_cborQueue(queueSize), rxdEnvelope(messageSize), txdEnvelope(messageSize) {
}

void Mailbox::enqueue(Envelope& msg) {
	_cborQueue.put(msg.message);
}

void Mailbox::dequeue(Envelope& msg) {
	_cborQueue.get(msg.message);
	msg.getHeader();
}

bool Mailbox::hasMessages() {
	return _cborQueue.hasData();
}

void Mailbox::handleMessages() {
	while (hasMessages()) {
		dequeue(rxdEnvelope); // load envelope and payload
//		ref => context => receive => handleMessage(rxdMessage);
	}
	// TODO handle timeouts
}
//_____________________________________________________________________
// ActorSystem
//
ActorSystem::ActorSystem(const char* name) :
		_name(name) {
}

//_____________________________________________________________________ Receiver
//
Receiver::Receiver(MsgClass msgClass, MessageMatcher matcher,
		MessageHandler handler) :
		_msgClass(msgClass), _matcher(matcher), _handler(handler) {
}

Receiver::Receiver(MsgClass msgClass, MessageHandler handler) :
		_msgClass(msgClass), _matcher(alwaysTrue), _handler(handler) {
}

void Receiver::handle(Envelope& msg) {
	_handler(msg);
}
bool Receiver::match(Envelope& msg) {
	if (_msgClass == msg.msgClass || _msgClass == AnyClass)
		return (_matcher(msg));
	return false;
}

Str& Receiver::toString(Str& s) {
	return s.format(" class : %s  ", UID.label(_msgClass));
}

//_____________________________________________________________________
// Receive
//
Receive::Receive() {
}

Receive& Receive::match(MsgClass msgClass, MessageHandler doSome) {
	Receiver* receiver = new Receiver(msgClass, Receiver::alwaysTrue, doSome);
	receivers.add(receiver);
	return *this;
}

Receive& Receive::build() {
	return *this;
}

//_____________________________________________________________________
// Receive

//_____________________________________________________________________
// ActorContext
//

ActorContext::ActorContext(ActorRef* self, ActorSystem* system,
		Mailbox* mailbox) :
		_self(self), _system(system), _mailbox(mailbox), _receive(&nullReceive), _timeout(
				UINT64_MAX) {

}

//_____________________________________________________________________
// Actor

LinkedList<Actor*> Actor::actors;

Actor::Actor(const char* name) :
		_name(name){
	ActorCell* cell=new ActorCell();
	ActorRef& self=cell->ref();
	ActorContext* ctx = new ActorContext(self,defaultActorSystem,defaultMailbox);
	context(*ctx);

	actors.add(this);
}

Actor::~Actor() {
}

Receive& Actor::receiveBuilder() {
	return context().receive();
}

void Actor::preStart() {
}

void Actor::postStop() {
}

void Actor::unhandled(Envelope& msg) {
	INFO("unhandled message for Actor : %s ", _name);
}

void Actor::setReceiveTimeout(uint32_t msec) {
}

uint32_t Actor::getReceiveTimeout() {
	return 0;
}

ActorRef Actor::sender() {
	return context().mailbox().rxdEnvelope.sender;
}

void Actor::system(ActorSystem& sys) {
	context().system(sys);
}

//________________________________________________________________________________

void ActorSystem::stop(ActorRef) {
}

uint32_t ActorCell::_actorCellCounter = 0;
ActorCell* ActorCell::_actorCells[MAX_ACTOR_CELLS];

ActorCell& ActorCell::get(uint32_t id) {
	ASSERT(id < MAX_ACTOR_CELLS);
	return *_actorCells[id];
}


ActorCell::ActorCell(){
}

inline Mailbox& ActorCell::mailbox() {
	return mailbox(ref.id);
}

inline Mailbox& ActorCell::mailbox(uint32_t id) {
	ASSERT(id < MAX_ACTOR_CELLS);
	return _actorCells[id]->create(;
}

inline ActorRef ActorCell::ref() {
	return _ref;
}

Mailbox deadLetterMailbox(1, 100);
ActorSystem defaultActorSystem("system");
