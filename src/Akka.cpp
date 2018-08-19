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

MsgClass AnyClass("$ANY");
Receive Receive::nullReceive;
ActorMsgBus bus;
Mailbox deadLetterMailbox("deadletterMailbox", 1, 100);
ActorSystem defaultActorSystem("system");

typedef void (*MsgHandler)(void);

//_____________________________________________________________________
// Actor

Actor::~Actor() {

}

LinkedList<AbstractActor*> AbstractActor::actors;

AbstractActor::AbstractActor() :
		_context(0) {
}

AbstractActor::~AbstractActor() {
}

void AbstractActor::context(ActorContext* ctx) {
	_context = ctx;
}

ActorRef AbstractActor::self() {
	return _context->self();
//	return ActorContext::context(this).self();
}

Receive& AbstractActor::receiveBuilder() {
//	context().receive(*(new Receive()));
	return context().receive();
}

ActorContext& AbstractActor::context() {
	return *_context;
//	return ActorContext::context(this);
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
/*
 void AbstractActor::handle(Envelope& msg) {
 context().receive().handle(msg);
 }*/
//____________________________________________________________ ActorRef
ActorRef ActorRef::noSender(0);
ActorRef ActorRef::anyActor(1);

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
	ActorContext* context = ActorContext::context(*this);
	if (context)
		return context->path();
	else
		return "unknown ref";
}

Mailbox& ActorRef::mailbox() {
	ActorContext* context = ActorContext::context(*this);
	if (context)
		return context->mailbox();
	else
		return deadLetterMailbox;
}

/*
 void ActorRef::ask(ActorRef dst, MsgClass type, Envelope& msg,
 uint32_t timeout) {
 }
 */
void ActorRef::tell(ActorRef src, MsgClass cls, const char* fmt, ...) {

	Mailbox& srcMailbox = src.mailbox();
	Mailbox& dstMailbox = mailbox();
	va_list args;
	va_start(args, fmt);
	srcMailbox.txdEnvelope.setHeader(src, *this, cls).message.vaddf(fmt, args);
	va_end(args);

	dstMailbox.enqueue(srcMailbox.txdEnvelope);
}

void ActorRef::tell(ActorRef sender, Envelope& envelope) {
	mailbox().enqueue(envelope);
}

void ActorRef::forward(Envelope& msg) {
	Mailbox& rcvMailbox = mailbox();
	Mailbox& sndMailbox = msg.receiver.mailbox();
	sndMailbox.txdEnvelope.setHeader(msg.sender, *this, msg.msgClass);
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
		sender(ActorRef::anyActor), receiver(ActorRef::anyActor), msgClass(
				AnyClass), id(0), message(size) {
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
			sender.path(), msgClass.name());
}

typedef bool (*MsgMatch)(Envelope& msg);

typedef std::function<void(Envelope&)> MessageHandler;

//_____________________________________________________________________ Mailbox

LinkedList<Mailbox*> mailboxes;

Mailbox::Mailbox(const char* name, uint32_t queueSize, uint32_t messageSize) :
		_name(name), _cborQueue(queueSize), rxdEnvelope(messageSize), txdEnvelope(
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
		ActorContext* context = ActorContext::context(rxdEnvelope.receiver);
		if (context) {
			context->receive().handle(rxdEnvelope);
		} else {
			WARN(" unknown destination ref %d ! trying remote. ",
					rxdEnvelope.receiver.id());
		}
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
		_name(name), _defaultMailbox(defaultMailbox), _deadLetterMailbox(
				deadLetterMailbox) {
}

uid_type ActorSystem::uniqueId(const char* name) {
	std::string s, t;
	s += Sys::hostname();
	s += "/";
	s += name;
	uid_type hash = H(s.c_str());
	if (UID.find(hash) == 0)
		return (new Uid(s.c_str()))->id();
	s += "#";
	int i = 1;
	while (true) {
		t = s;
		t += i;
		hash = H(t.c_str());
		if (UID.find(hash) == 0)
			return (new Uid(t.c_str()))->id();
		else
			i++;
	};
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
	return s.format(" class : %s  ", _msgClass.name());
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

void Receive::handle(Envelope& envelope) {
	receivers.forEach([&envelope](Receiver* receiver) {
		if (receiver->match(envelope)) {
			receiver->handle(envelope);
		}
	});

}

//_____________________________________________________________________
// Receive

//_________________________________________________________________________ ActorContext
//
LinkedList<ActorContext*> ActorContext::_actorContexts;

ActorContext::ActorContext(uid_type id, ActorRef self, AbstractActor& actor,
		ActorSystem& system, Mailbox& mailbox, Receive& receive) :
		_id(id), _self(self), _actor(actor), _system(system), _mailbox(mailbox), _receive(
				receive) {
	_receiveTimeout = UINT64_MAX;
	_actorContexts.add(this);
}

ActorContext* ActorContext::context(ActorRef& ref) {
	return _actorContexts.findFirst([&ref](ActorContext* ctx) {
		return ref.id()==ctx->id();
	});
}

ActorContext& ActorContext::context(AbstractActor* actor) {
	return *_actorContexts.findFirst([actor](ActorContext* ctx) {
		return & ctx->_actor == actor;
	});
}

uid_type ActorContext::id() {
	return _id;
}

const char* ActorContext::path() {
	return UID.label(_id);
}

Mailbox& ActorContext::mailbox() {
	return _mailbox;
}

Receive& ActorContext::receive() {

	return _receive;
}

void ActorContext::receive(Receive& r) {
	_receive = r;
}

ActorRef ActorContext::self() {
	return _self;
}

void ActorContext::self(ActorRef& ref) {
	ASSERT(ref.id() == _self.id());
	_self = ref;
}

class DeadLetterActor: public AbstractActor {

public:
	DeadLetterActor() {
	}
	~DeadLetterActor() {
	}
	Receive& createReceive() {
		return receiveBuilder().match(AnyClass,
				[this](Envelope& msg) {
					INFO(" DeadLetter Actor from '%s' to '%s' msg '%s' ",msg.sender.path(),msg.receiver.path(),msg.msgClass.name());

				}).build();
	}
} deadLetterActor;

