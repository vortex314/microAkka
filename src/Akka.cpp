/*
 * Akka.cpp
 *
 *  Created on: 2-aug.-2018
 *      Author: D-MG61DD
 */

#include "Akka.h"

//============================================================================
// Name        : akkaMicro.cpp
// Author      : Lieven
// Version     :
// Copyright   : Enjoy the source
// Description : Akka alike framework in C++ for embedded systems : low RAM
//============================================================================

#include <Cbor.h>
#include <CborQueue.h>
#include <LinkedList.hpp>
#include <functional>
#include <stdarg.h>
#include <stdint.h>

//_____________________________________________- STATIC

MsgClass AnyClass("$ANY");
// ActorMsgBus bus;
// Mailbox deadLetterMailbox("deadletterMailbox", 1, 100);
ActorSystem defaultActorSystem("system");
MsgClass ReceiveTimeout("ReceiveTimeout");
MsgClass TimerExpired("TimerExpired");
ActorRef NoSender("NoSender");
ActorRef AnyActor("AnyActor");
MessageDispatcher defaultDispatcher;
Props defaultProps(defaultDispatcher, defaultMailbox);
Envelope NoMessage(NoSender, NoSender, "");

typedef void (*MsgHandler)(void);

UidType::UidType(const char* name) {
	_id = UID.hash(name);
	//    _name = label();
}

UidType::UidType(uint16_t id) :
		_id(id) { /*_name = label();*/
}

bool UidType::operator==(UidType v) {
	return _id == v._id;
}

const char* UidType::label() {
	return UID.label(_id);
}

// bool UidType::hasLabel() { return UID.find(_id) != 0; }

uid_type UidType::id() {
	return _id;
}

void UidType::id(uid_type id) {
	_id = id;
	//    _name = label();
}

//_____________________________________________________________________
// Actor

LinkedList<Actor*> Actor::_actors;

Actor::Actor() :
		_context() {
}

Actor::~Actor() {
}

ActorRef& Actor::self() {
	return _context->self();
}
;

Receive& Actor::receiveBuilder() {
	return *(new Receive());
}

ActorContext& Actor::context() {
	return *_context;
}
TimerScheduler& Actor::timers() {
	return _context->timers();
}
;

void Actor::unhandled(Envelope& msg) {
	INFO("unhandled message for Actor : %s ", self().path());
}

ActorRef& Actor::sender() {
	return _context->sender();
}

//____________________________________________________________ ActorRef

LinkedList<ActorRef*> ActorRef::_actorRefs;

ActorRef* ActorRef::lookup(uid_type id) {
	return _actorRefs.findFirst([id](ActorRef* t) {
		//        INFO(" %d : %d %s:%s ", t->id(), id,
		//        Uid::label(t->id()),Uid::label(id));
			return t->id() == id;
		});
}

ActorRef::ActorRef(UidType id) :
		_id(id) {
	INFO(" created ActorRef %s", id.label());
	_actorRefs.add(this);
	_cell = 0;
}

bool ActorRef::isLocal() {
	return _cell == 0;
}

ActorRef::ActorRef() :
		_id(ActorRef::noSender.id()) {
}

const char* ActorRef::path() {
	return _id.label();
}

Mailbox& ActorRef::mailbox() {
	return *_mailbox;
}
void ActorRef::mailbox(Mailbox& mb) {
	_mailbox = &mb;
}
ActorRef ActorRef::noSender = ActorRef("noSender");
uid_type ActorRef::id() {
	return _id.id();
}
;
bool ActorRef::operator==(ActorRef& v) {
	return _id == v._id;
}
//____________________________________________________________________________
// LocalActorRef

/*
 void ActorRef::ask(ActorRef dst, MsgClass type, Envelope&
 msg,!array.is<char*>(0) uint32_t timeout) {
 }
 */
Envelope env(1024);
void ActorRef::tell(ActorRef src, MsgClass cls, const char* fmt, ...) {

	env.message.clear();
	va_list args;
	va_start(args, fmt);
	env.header(*this, src, cls);
	env.message.vaddf(fmt, args);
	va_end(args);
	tell(src, env);
}

void ActorRef::tell(ActorRef src, MsgClass cls, uint16_t id, const char* fmt,
		...) {
	env.message.clear();
	va_list args;
	va_start(args, fmt);
	env.header(*this, src, cls, id);
	env.message.vaddf(fmt, args);
	va_end(args);
	tell(src, env);
}

void ActorRef::tell(ActorRef sender, Envelope& envelope) {
	mailbox().enqueue(envelope);
}

void ActorRef::forward(Envelope& msg) {
	Mailbox& mb = mailbox();
	msg.receiver = this;
	mb.enqueue(msg);
}
void ActorRef::cell(ActorCell* cell) {
	_cell = cell;
}

ActorCell* ActorRef::cell() {
	return _cell;
}

//____________________________________________________________ ActorSelection

ActorSelection::ActorSelection(UidType id) :
		ActorRef(id) {
}

//______________________________________________________________________
// Envelope
//

Envelope::Envelope(uint32_t size) :
		sender(&AnyActor), receiver(&AnyActor), msgClass(AnyClass), id(0), message(
				size) {
}

Envelope::Envelope(ActorRef& snd, ActorRef& rcv, MsgClass clz) :
		sender(&snd), receiver(&rcv), msgClass(clz), id(newId()), message(32) {
}

uid_type Envelope::idCounter = 0;

uint32_t Envelope::newId() {
	return idCounter++;
}

Envelope& Envelope::header(ActorRef& rcv, ActorRef& snd, MsgClass clz) {
	sender = &snd;
	receiver = &rcv;
	msgClass = clz;
	id = newId();
	return *this;
}

Envelope& Envelope::header(ActorRef& rcv, ActorRef& snd, MsgClass clz,
		uint16_t i) {
	sender = &snd;
	receiver = &rcv;
	msgClass = clz;
	id = i;
	return *this;
}

bool Envelope::scanf(const char* fmt, ...) {
	bool b = false;
	va_list args;
	va_start(args, fmt);
	b = message.vscanf(fmt, args);
	va_end(args);
	return b;
}

void Envelope::vaddf(const char* fmt, va_list args) {
	message.vaddf(fmt, args);
}

void Envelope::addf(const char* fmt, ...) {
	va_list args;
	va_start(args, fmt);
	message.vaddf(fmt, args);
	va_end(args);
}

Str& Envelope::toString(Str& s) {
	return s.format(" dst : %s , src : %s , class : %s  ", receiver->path(),
			sender->path(), msgClass.label());
}

typedef bool (*MsgMatch)(Envelope& msg);

typedef std::function<void(Envelope&)> MessageHandler;
//____________________________________________________________ MessageQueue

MessageQueue::MessageQueue(int queueSize, int messageSize) :
		_cborQueue(queueSize), _cbor(messageSize) {
}

bool MessageQueue::hasMessages() {
	return _cborQueue.hasData();
}

void MessageQueue::enqueue(Envelope& msg) {
	_cbor.clear();
	_cbor.add(msg.receiver->id());
	_cbor.add(msg.sender->id());
	_cbor.add(msg.msgClass.id());
	_cbor.add(msg.id);
	_cbor.append(msg.message);
	_cborQueue.put(_cbor);
}

void MessageQueue::dequeue(Envelope& msg) {
	_cborQueue.get(_cbor);
	uid_type uid;
	_cbor.get(uid);
	msg.receiver = ActorRef::lookup(uid);
	_cbor.get(uid);
	msg.sender = ActorRef::lookup(uid);
	_cbor.get(uid);
	msg.msgClass.id(uid);
	_cbor.get(msg.id);
	msg.message.clear();
	while (_cbor.hasData()) {
		msg.message.write(_cbor.read());
	}
}

//_____________________________________________________________________ Mailbox

LinkedList<Mailbox*> Mailbox::_mailboxes;

Mailbox::Mailbox(const char* name, uint32_t queueSize, uint32_t messageSize) :
		MessageQueue(queueSize, messageSize), _name(name) {
	_mailboxes.add(this);
//    INFO(" added mailbox : %s ", this->_name);
}

LinkedList<Mailbox*>& Mailbox::mailboxes() {
	return _mailboxes;
}
//_______________________________________________________________ ActorSystem
//
ActorSystem::ActorSystem(const char* name) :
		UidType(name), _defaultMailbox(&defaultMailbox), _name(name)
		//    _deadLetterMailbox(&deadLetterMailbox),
				, _defaultDispatcher(&defaultDispatcher) {
}

UidType ActorSystem::uniqueId(const char* name) {
	std::string s, t;
	s += Sys::hostname();
	s += "/";
	s += name;
	UidType hash = H(s.c_str());
	if (hash.label() == 0)
		return UidType(s.c_str());
	s += "#";
	int i = 1;
	while (true) {
		t = s;
		t += i;
		hash = H(t.c_str());
		if (hash.label() == 0)
			return UidType(t.c_str());
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

inline void Receiver::onMessage(Envelope& msg) {
	_handler(msg);
}
bool Receiver::match(Envelope& msg) {
	if (_msgClass == msg.msgClass || _msgClass == AnyClass)
		return (_matcher(msg));
	return false;
}

Str& Receiver::toString(Str& s) {
	return s.format(" class : %s  ", _msgClass.label());
}
//_____________________________________________ Timer

Timer::Timer(UidType key, bool active, bool periodic, uint64_t interval) :
		UidType(key), _active(active), _periodic(periodic), _interval(interval) {
	load();
}
bool Timer::active() {
	return _active;
}
void Timer::active(bool t) {
	_active = t;
}
uid_type Timer::key() {
	return id();
}
void Timer::load() {
	_expiresAt = Sys::millis() + _interval;
}
void Timer::reload() {
	if (_periodic) {
		load();
	} else {
		_active = false;
	}
}
bool Timer::expired() {
	return Sys::millis() > _expiresAt;
}
uint64_t Timer::expiresAt() {
	if (_active)
		return _expiresAt;
	return UINT64_MAX;
}
void Timer::set(bool active, bool periodic, uint32_t msec) {
	_active = active;
	_periodic = periodic;
	_interval = msec;
	load();
}
//__________________________________________________________ TimerScheduler

TimerScheduler::TimerScheduler() {
}

Timer* TimerScheduler::find(uid_type key) {
	return _timers.findFirst([key](Timer* t) {return t->key() == key;});
}

Timer* TimerScheduler::findNextTimeout() {
	uint64_t nextTimeout = UINT64_MAX;
	Timer* t = 0;
	_timers.forEach([&nextTimeout, &t](Timer* timer) {
		if(timer->active() && timer->expiresAt() < nextTimeout) {
			t = timer;
			nextTimeout = timer->expiresAt();
		}
	});
	return t;
}

uid_type TimerScheduler::startPeriodicTimer(UidType key, MsgClass cls,
		uint32_t msec) {
	Timer* timer = find(key.id());

	if (timer == 0) {
		_timers.add(new Timer(key, true, true, msec));
	} else {
		timer->set(true, true, msec);
	}
	return key.id();
}
uid_type TimerScheduler::startSingleTimer(UidType key, MsgClass,
		uint32_t msec) {
	Timer* timer = find(key.id());

	if (timer == 0) {
		_timers.add(new Timer(key, true, false, msec));
	} else {
		timer->set(true, false, msec);
	}
	return key.id();
}
void TimerScheduler::cancel(uid_type key) {
	Timer* timer = find(key);
	if (timer)
		timer->active(false);
	else
	WARN(" timer.cancel()  key not found : %s ", UID.label(key));
}
void TimerScheduler::cancelAll() {
	_timers.forEach([](Timer* t) {t->active(false);});
}
bool TimerScheduler::isTimerActive(uid_type key) {
	Timer* timer = _timers.findFirst([key](Timer* t) {return t->key() == key;});
	if (timer)
		return timer->active();
	else {
		WARN(" timer.isTimerActive()  key not found : %s ", UID.label(key));
		return false;
	}
}

//_____________________________________________________________________
// Receive
//
Receive::Receive() {
}

Receive& Receive::match(MsgClass msgClass, MessageHandler doSome) {
	Receiver* receiver = new Receiver(msgClass, Receiver::alwaysTrue, doSome);
	_receivers.add(receiver);
	return *this;
}

Receive& Receive::build() {
	return *this;
}

void Receive::onMessage(Envelope& envelope) {
	_receivers.forEach([&](Receiver* receiver) {
		if(receiver->match(envelope)) {
			envelope.message.offset(0);
			receiver->onMessage(envelope);
		}
	});
}

Receive nullReceive;

//______________________________________________________________ ActorCell
//


LinkedList<ActorCell*> ActorCell::_actorCells;

ActorCell::ActorCell(ActorSystem& system, ActorRef& ref, Mailbox& mailbox,
		MessageDispatcher& dispatcher) :
		_mailbox(mailbox), _system(system), _dispatcher(dispatcher), _self(ref) {
	_currentMessage = 0;
	_timers = 0;
	_lastReceive = UINT64_MAX;
	_inactivityPeriod = UINT32_MAX;
	_receive=0;
	_prevReceive=0;
	_actorCells.add(this);
}

const char* ActorCell::path() {
	return _self.path();
}
Mailbox& ActorCell::mailbox() {
	return _mailbox;
}
ActorRef& ActorCell::self() {
	return _self;
}

ActorCell* ActorCell::cellFor(ActorRef* ref) {
	return ref->cell();
}

void ActorCell::invoke(Envelope& msg) {
	_currentMessage = &msg;
	_receive->onMessage(msg);
}

ActorRef& ActorCell::sender() {
	return *_currentMessage->sender;
}
ActorSystem& ActorCell::system() {
	return _system;
}

void ActorCell::become(Receive& r, bool discardOld) {
	_receive = &r;
}
void ActorCell::unbecome() {
	_receive = &nullReceive;
}

void ActorCell::setReceiveTimeout(uint32_t msec) {
	_inactivityPeriod = msec;
	_lastReceive = Sys::millis();
}

uint32_t ActorCell::receiveTimeout() {
	return _inactivityPeriod;
}

void ActorCell::resetReceiveTimeout() {
	_lastReceive = Sys::millis();
}

bool ActorCell::hasReceiveTimedOut() {
	return Sys::millis() > (_lastReceive + _inactivityPeriod);
}

TimerScheduler& ActorCell::timers() {
	if (_timers == 0)
		_timers = new TimerScheduler();
	return *_timers;
}

bool ActorCell::hasTimers() {
	return _timers != 0;
}

LinkedList<ActorCell*>& ActorCell::actorCells() {
	return _actorCells;
}

//_____________________________________________
/*
 class DeadLetterActor : public Actor
 {

 public:
 DeadLetterActor()
 : Actor()
 {
 }
 ~DeadLetterActor()
 {
 }
 Receive& createReceive()
 {
 return receiveBuilder()
 .match(AnyClass,
 [this](Envelope& msg) {
 INFO(" DeadLetter Actor from '%s' to '%s' msg '%s' ", msg.sender->path(), msg.receiver->path(),
 msg.msgClass.label());
 })
 .build();
 }
 }; // deadLetterActor;*/

MessageDispatcher::MessageDispatcher() {
	_unhandledCell = 0;
}


void MessageDispatcher::attach(Mailbox& mailbox) {
	_mailboxes.add(&mailbox);
}
void MessageDispatcher::attach(ActorCell& cell) {
	_actorCells.add(&cell);
}
void MessageDispatcher::unhandled(ActorCell* cell) {
	_unhandledCell = cell;
}

bool doneSomething = false;

bool busy() {
	return doneSomething == true;
}

void idle() {
	doneSomething = false;
}

void working() {
	doneSomething = true;
}

void MessageDispatcher::execute() {
	static Envelope rxdEnvelope(1024);
	working();
	while (busy()) {
		idle();
		_mailboxes.forEach([this](Mailbox* mb) { // take a message from each mailbox if needed
					if(mb->hasMessages()) {
						mb->dequeue(rxdEnvelope); // load envelope and payload
						/*                   INFO(" >> receiver : %s [%d]",
						 rxdEnvelope.receiver->path(),
						 rxdEnvelope.receiver->id());*/
						ActorCell* cell = ActorCell::cellFor(rxdEnvelope.receiver);
						if(cell) {
							cell->invoke(rxdEnvelope);
							cell->resetReceiveTimeout();
						} else {
							if(mb != &remoteMailbox) {
								remoteMailbox.enqueue(rxdEnvelope);
							} else {
								if(_unhandledCell) {
									_unhandledCell->invoke(rxdEnvelope);
								} else {
									INFO(" no Receive found  %s->%s:%s ", rxdEnvelope.sender->path(),
											rxdEnvelope.receiver->path(), rxdEnvelope.msgClass.label());
								}
							}
							//                   deadLetterActor.context().invoke(rxdEnvelope);
						}
						working();
					}
				});

		_actorCells.forEach([](ActorCell* ac) { // check timeouts
					Timer* timer;
					if(ac->hasTimers() && (timer = ac->timers().findNextTimeout()) && (timer->expiresAt() < Sys::millis())) {
						timer->reload(); // retrigger now message will be send
						Envelope timerExpired(NoSender, ac->self(), TimerExpired);
						timerExpired.message.add(timer->key());
						ac->self().tell(NoSender, timerExpired);
						working();
					}
					if(ac->hasReceiveTimedOut()) {
						Envelope receiveTimeout(NoSender, ac->self(), ReceiveTimeout); //TBD
						ac->self().tell(NoSender, receiveTimeout);
						ac->resetReceiveTimeout();
						working();
					}
				});
	}
}

// void arduinoLoop() { defaultDispatcher.execute(); }
