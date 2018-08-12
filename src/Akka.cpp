/*
 * Akka.cpp
 *
 *  Created on: 2-aug.-2018
 *      Author: D-MG61DD
 */

#include <Akka.h>

//============================================================================
// Name        : akkaMicro.cpp
// Author      : Lieven
// Version     :
// Copyright   : Enjoy the source
// Description : Akka alike framework in C++ for embedded systems : low RAM
//============================================================================

#include <functional>
#include <stdarg.h>
#include <stdint.h>


using namespace std;

#include <Cbor.h>
#include <CborQueue.h>
#include <LinkedList.hpp>
#include <Log.h>
#include <Str.h>
#include <Uid.h>

//_____________________________________________- STATIC
ActorRef AnyActor(0);
ActorRef NoSender(1);
uid_type AnyClass = 0;
Mailbox deadLetter(1, 10);
Receive nullReceive;

typedef uid_type MsgClass;
typedef void (*MsgHandler)(void);

//_____________________________________________________________________
// Actor

LinkedList<AbstractActor*> AbstractActor::actors;

AbstractActor::AbstractActor() :
		 _context(*(new ActorContext())) {
	ActorCell* cell = new ActorCell();
	cell->mailbox(defaultMailbox);
	_context.self(cell->ref());
	actors.add(this);
}

AbstractActor::~AbstractActor() {
}

ActorRef AbstractActor::self() {
	return _context.self();
}

Receive& AbstractActor::receiveBuilder() {
//	context().receive(*(new Receive()));
	return context().receive();
}

ActorContext& AbstractActor::context() {
	return _context;
}

void AbstractActor::preStart() {
}

void AbstractActor::postStop() {
}

void AbstractActor::unhandled(Envelope& msg) {
	INFO("unhandled message for Actor : %s ", self().path());
}

ActorRef AbstractActor::sender() {
	return context().mailbox().rxdEnvelope.sender;
}

void AbstractActor::handle(Envelope& msg) {
	context().receive().handle(msg);
}
//____________________________________________________________ ActorRef

ActorRef::ActorRef(uid_type id) :
		_id(id) {
}

bool ActorRef::operator==(ActorRef& dst) {
	return (_id == dst._id);
}

inline uint16_t ActorRef::id() {
	return _id;
}

void ActorRef::id(uint16_t id) {
	_id = id;
}

const char* ActorRef::path() {
	// TODO path
	return ActorCell::cell(*this).path();
}

void ActorRef::path(const char* p){
	ActorCell::cell(*this).path(p);
}

Mailbox& ActorRef::mailbox() {
	return ActorCell::cell(*this).mailbox();
}

void ActorRef::ask(ActorRef dst, MsgClass type, Envelope& msg,
		uint32_t timeout) {
}

void ActorRef::tell(ActorRef src, MsgClass cls, const char* fmt, ...) {

	Mailbox& srcMailbox = src.mailbox();
	Mailbox& dstMailbox = mailbox();
	va_list args;
	va_start(args, fmt);
	srcMailbox.txdEnvelope.setHeader(src, _id, cls).message.vaddf(fmt, args);
	va_end(args);

	dstMailbox.enqueue(srcMailbox.txdEnvelope);
}

void ActorRef::forward(Envelope& msg) {
	Mailbox& rcvMailbox = mailbox();
	Mailbox& sndMailbox = msg.receiver.mailbox();
	sndMailbox.txdEnvelope.setHeader(msg.sender, _id, msg.msgClass);
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
		sender(AnyActor), receiver(AnyActor), msgClass(AnyClass), id(0), message(
				size) {
}

Envelope& Envelope::setHeader(ActorRef snd, ActorRef rcv, MsgClass clz) {
	message.offset(0);
	sender = snd;
	receiver = rcv;
	msgClass = clz;
	id = newId();
	message.addf("2222", snd.id(), rcv.id(), clz, id);
	return *this;
}

bool Envelope::getHeader() {
	uint16_t s, r;
	bool b = message.scanf("2222", &s, &r, &msgClass, &id);
	if (b) {
		sender.id(s);
		receiver.id(r);
	}
	return b;
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
	return s.format(" dst : %s , src : %s , class : %s  ", receiver.path(),
			sender.path(), UID.label(msgClass));
}

typedef bool (*MsgMatch)(Envelope& msg);

typedef std::function<void(Envelope&)> MessageHandler;

//_____________________________________________________________________ Mailbox

LinkedList<Mailbox*> mailboxes;

Mailbox::Mailbox(uint32_t queueSize, uint32_t messageSize) :
		_cborQueue(queueSize), rxdEnvelope(messageSize), txdEnvelope(
				messageSize) {
}

void Mailbox::enqueue(Envelope& msg) {
	_cborQueue.put(msg.message);
}

void Mailbox::dequeue(Envelope& msg) {
	_cborQueue.get(msg.message);
}

bool Mailbox::hasMessages() {
	return _cborQueue.hasData();
}

void Mailbox::handleMessages() {
	while (hasMessages()) {
		dequeue(rxdEnvelope); // load envelope and payload
		rxdEnvelope.getHeader();
		ActorContext::context(rxdEnvelope.receiver).receive().handle(
				rxdEnvelope);
#ifdef ARDUINO
		::yield();
#endif
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

inline void Receiver::handle(Envelope& msg) {
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
Receive::Receive() : v(vCount++){
}

Receive& Receive::match(MsgClass msgClass, MessageHandler doSome) {
	Receiver* receiver = new Receiver(msgClass, Receiver::alwaysTrue, doSome);
	receivers.add(receiver);
	return *this;
}

Receive& Receive::build() {
	return *this;
}

void Receive::handle(Envelope& envelope) {
	receivers.forEach([&envelope](Receiver* receiver) {
		if (receiver->match(envelope)) {
			receiver->handle(envelope);
		}
	});

}

//_____________________________________________________________________
// Receive

//_____________________________________________________________________
// ActorContext
//

ActorContext::ActorContext(ActorRef& self, ActorSystem& system,
		Mailbox& mailbox, Receive* receive) :
		_self(self), _system(system), _mailbox(mailbox), _receive(receive), _timeout(
		UINT64_MAX) {
	_idx = self.id();
}


//________________________________________________________________________________

void ActorSystem::stop(ActorRef) {
}

uint32_t ActorCell::_actorCellCounter = 0;
ActorCell* ActorCell::_actorCells[MAX_ACTOR_CELLS];

ActorCell::ActorCell() :
		_idx(_actorCellCounter), _ref(*(new ActorRef(_idx))), _mailbox(
				defaultMailbox),_path("NO-PATH") {
	_actorCells[_idx] = this;
//	_ref =  *(new ActorRef(_idx));
	_actorCellCounter++;
}

ActorCell& ActorCell::cell(ActorRef ref) {
	return *(_actorCells[ref.id()]);
}

ActorRef& ActorCell::ref() {
	return _ref;
}

inline Mailbox& ActorCell::mailbox() {
	return _mailbox;
}

void ActorCell::mailbox(Mailbox& mb) {
	_mailbox = mb;
}

void ActorCell::path(const char* p){
	_path=p;
}

const char* ActorCell::path(){
	return _path;
}

//_________________________________________________________________________ ActorContext
//

uint32_t ActorContext::_actorContextCounter = 0;
ActorContext* ActorContext::_actorContexts[MAX_ACTOR_CELLS];

ActorContext::ActorContext() :
		_idx(_actorContextCounter), _self(*(new ActorRef(_idx))), _system(
				defaultActorSystem), _mailbox(defaultMailbox), _receive(&nullReceive), _timeout(
		UINT64_MAX) {
	_actorContexts[_idx] = this;
	_actorContextCounter++;
}

ActorContext& ActorContext::context(ActorRef& ref) {
	return *_actorContexts[ref.id()];
}

Mailbox& ActorContext::mailbox() {
	return _mailbox;
}

Receive& ActorContext::receive() {
	ASSERT(_receive!=0);
	return *_receive;
}

void ActorContext::receive(Receive& r) {
	_receive = &r;
}

ActorRef ActorContext::self() {
	return _self;
}

void ActorContext::self(ActorRef& ref) {
	ASSERT(ref.id() == _self.id());
	_self = ref;
}

void ActorContext::system(ActorSystem& sys) {
	_system = sys;
}

uint32_t Receive::vCount=0;

Mailbox deadLetterMailbox(1, 100);
ActorSystem defaultActorSystem("system");
