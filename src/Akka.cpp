//============================================================================
// Name        : akkaMicro.cpp
// Author      : Lieven
// Version     :
// Copyright   : Enjoy the source
// Description : Akka alike framework in C++ for embedded systems : low RAM
//============================================================================
#include "Akka.h"

class Static {
		static Uid uid;
		static ActorRef actorRef;
		static ActorCell cell;
		static Mailbox mailbox;
		static ActorSystem actorSystem;
		static Log log;
		static MsgClass msgClass;
} ____first;

//_____________________________________________ STATIC
const MsgClass ReceiveTimeout("ReceiveTimeout");
const MsgClass TimerExpired("TimerExpired");
const MsgClass PoisonPill("PoisonPill");
const MsgClass AnyClass("AnyClass");
const MsgClass Properties("Properties");
const MsgClass PropertiesReply("PropertiesReply");

const MsgClass Rxd("Rxd");
const MsgClass Txd("Txd");

ActorRef& NoSender() {
	static ActorRef NoSender("NoSender");
	return NoSender;
}
Receive NullReceive;

// MsgClass& AnyClass;
// ActorMsgBus bus;
// Mailbox deadLetterMailbox("deadletterMailbox", 1, 100);
// ActorSystem defaultActorSystem("system");



// void* Uid::object(uid_type id);
// Envelope& NoMessage;

typedef void (*MsgHandler)(void);

//_______________________________________________ uid_type
//

//_________________________________________________ MsgClass

//_________________________________________________ Actor

list<Actor*> Actor::_actors;

Actor::Actor() : _context() {}

Actor::~Actor() {}

ActorRef& Actor::self() { return _context->self(); }

Receive& Actor::receiveBuilder() { return *(new Receive()); }

ActorContext& Actor::context() { return *_context; }

TimerScheduler& Actor::timers() { return _context->timers(); }

void Actor::unhandled(Envelope& msg) {
	INFO("unhandled message for Actor : %s ", self().path());
}

ActorRef& Actor::sender() { return _context->sender(); }

Envelope& Actor::txdEnvelope() { return context().dispatcher().txdEnvelope(); }

//_______________________________________________ ActorRef

ActorRef::ActorRef() : Uid(NoSender()) {}

unordered_map<uid_type, ActorRef*> ActorRef::_actorRefs;

ActorRef* ActorRef::lookup(Uid uid) {
	std::unordered_map<uid_type, ActorRef*>::const_iterator got =
	    _actorRefs.find(uid.id());
	if (got != _actorRefs.end())
		return got->second;
	return 0;
}

ActorRef::ActorRef(Uid uid, Mailbox* mailbox) : Uid(uid) {
	INFO(" created ActorRef '%s' = %d", uid.label(),uid.id());
	if (_actorRefs.find(uid.id()) == _actorRefs.end())
		_actorRefs.emplace(uid.id(), this);
	_cell = 0;
	_mailbox = mailbox;
}

ActorRef::ActorRef(Uid uid) : Uid(uid) {
	INFO(" created ActorRef '%s' = %d", uid.label(),uid.id());
	if (_actorRefs.find(uid.id()) == _actorRefs.end())
		_actorRefs.emplace(uid.id(), this);
	_cell = 0;
	_mailbox = 0;
}

bool ActorRef::isLocal() { return _cell == 0; }

const char* ActorRef::path() { return label(); }

Mailbox& ActorRef::mailbox() { return *_mailbox; }
void ActorRef::mailbox(Mailbox& mb) { _mailbox = &mb; }

bool ActorRef::operator==(ActorRef& v) { return id() == v.id(); }
//______________________________________________ LocalActorRef

void ActorRef::tell(ActorRef& src,  Msg& msg) {

	msg.add(UID_SRC,src.id());
	msg.add(UID_DST,this->id());
	msg.add(UID_ID,Envelope::newId());

	if (_mailbox != 0)
		_mailbox->enqueue(msg);
	else {
		WARN(" no mailbox attached to ActorRef %s ", label());
	}
}
/*
void ActorRef::tell(ActorRef& src, MsgClass cls, uint16_t id, const char* fmt,
                    ...) {
	Envelope env(256);

	env.clear();
	va_list args;
	va_start(args, fmt);
	env.header(*this, src, cls, id);
//	env.vaddf(fmt, args);
	va_end(args);
	tell(src, env);
}
*/
void ActorRef::tell(Envelope& envelope) {
	if (_mailbox != 0)
		_mailbox->enqueue(envelope);
	else {
		WARN(" no mailbox attached to ActorRef %s ", label());
	}
}

void ActorRef::forward(Envelope& envelope) {
	envelope.receiver = this;
	if (_mailbox != 0)
		_mailbox->enqueue(envelope);
	else {
		WARN(" no mailbox attached to ActorRef %s ", label());
	}
}
void ActorRef::cell(ActorCell* cell) { _cell = cell; }

ActorCell* ActorRef::cell() { return _cell; }

//____________________________________________________________ ActorSelection

ActorSelection::ActorSelection(Uid id) : ActorRef(id, 0) {}

//____________________________________________________________ Envelope
//

Envelope::Envelope(uint32_t size)
	: Msg(size), sender(&NoSender()), receiver(&NoSender()), msgClass(AnyClass), id(0) {}

Envelope::Envelope(ActorRef& snd, ActorRef& rcv, MsgClass clz)
	: Msg(100),sender(&snd), receiver(&rcv), msgClass(clz), id(newId()) {}

Envelope::Envelope(ActorRef& snd, MsgClass clz)
	:Msg(100), sender(&snd), receiver(&NoSender()), msgClass(clz), id(newId()) {}

uint32_t Envelope::_idCounter = 0;

uint32_t Envelope::newId() { return _idCounter++; }

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
/*
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

string& Envelope::toString(string& s) {
	string_format(s, " dst : %s , src : %s , class : %s  ", receiver->path(),
	              sender->path(), msgClass.label());
	s= toString();
	return s;
}
*/
typedef bool (*MsgMatch)(Envelope& msg);

typedef std::function<void(Envelope&)> MessageHandler;
//________________________________________________________ MessageQueue
//
MessageQueue::MessageQueue(int queueSize, int messageSize)
	: _queue(queueSize), _txd(messageSize), _rxd(messageSize),
	  _sema(Semaphore::create()) {
}

bool MessageQueue::hasMessages() { return _queue.hasData(1); }

void MessageQueue::enqueue(Envelope& msg) {
	uid_type uid;
	_sema.wait();
	_txd.clear();
	_txd.add(UID_DST,msg.receiver->id());
	_txd.add(UID_SRC,msg.sender->id());
	_txd.add(UID_CLS,msg.msgClass.id());
	_txd.add(UID_ID,msg.id);
	_txd.add(msg);
	assert(_txd.get(UID_SRC,uid)==0);
	assert(_txd.get(UID_CLS,uid)==0);

	_queue.enqueue(_txd);
	_sema.release();
}

void MessageQueue::enqueue(Msg& msg) {
	uid_type uid;
	_sema.wait();
	assert(msg.get(UID_SRC,uid)==0);
	assert(msg.get(UID_CLS,uid)==0);

	_queue.enqueue(msg);
	_sema.release();
}

void MessageQueue::dequeue(Envelope& msg) {
	_sema.wait();
	_queue.dequeue(_rxd);
//	INFO(" dequeued %d words.",_rxd.size());

	uid_type uid;
	if ( _rxd.get(UID_DST,uid)==0 ) {
		msg.receiver = ActorRef::lookup(uid);
	} else {
		msg.receiver = &NoSender();
	};
	assert(msg.receiver!=0);
	assert( _rxd.get(UID_SRC,uid) ==0);
	msg.sender = ActorRef::lookup(uid);
	assert(msg.sender !=0);
	assert ( _rxd.get(UID_CLS,uid)==0);
	msg.msgClass = MsgClass(uid);
	_rxd.get(UID_ID,msg.id);
	msg.clear();
	msg.add(_rxd);
	_sema.release();
}

//____________________________________________________________ Mailbox

list<Mailbox*> Mailbox::_mailboxes;

Mailbox::Mailbox(const char* name, uint32_t queueSize, uint32_t messageSize)
	: MessageQueue(queueSize, messageSize), _name(name) {
	_mailboxes.push_back(this);
}

list<Mailbox*>& Mailbox::mailboxes() { return _mailboxes; }
//_______________________________________________________ ActorSystem
//
ActorSystem::ActorSystem(const char* name, MessageDispatcher& defaultDispatcher,
                         Mailbox& defaultMailbox)
	: Uid(name), _name(name), _defaultMailbox(&defaultMailbox),
	  _defaultDispatcher(&defaultDispatcher),
	  _defaultProps(defaultDispatcher, defaultMailbox) {
	Uid::add(AKKA_DST);
	Uid::add(AKKA_SRC);
	Uid::add(AKKA_CLS);
	Uid::add(AKKA_ID);
	Uid::add(AKKA_TIMER);
}

Uid ActorSystem::uniqueId(const char* name) {
	string s, t;
	s += _name;
	s += "/";
	s += name;
	uid_type hash = H(s.c_str());
	if (Uid::label(hash) == 0) {
		return Uid(s.c_str());
	}
	s += "#";
	int i = 1;
	while (true) {
		string_format(t, "%s%d", s.c_str(), i);
		hash = H(t.c_str());
		if (Uid::label(hash) == 0)
			return Uid(t.c_str());
		else
			i++;
	};
}

//___________________________________________________________ Receiver
//
Receiver::Receiver(MsgClass msgClass, MessageMatcher matcher,
                   MessageHandler handler)
	: _msgClass(msgClass), _matcher(matcher), _handler(handler) {}

Receiver::Receiver(MsgClass msgClass, MessageHandler handler)
	: _msgClass(msgClass), _matcher(alwaysTrue), _handler(handler) {}

inline void Receiver::onMessage(Envelope& msg) { _handler(msg); }
bool Receiver::match(Envelope& msg) {
	if (_msgClass == msg.msgClass || _msgClass == AnyClass)
		return (_matcher(msg));
	return false;
}

string& Receiver::tostringing(string& s) {
	string_format(s, " class : %s  ", _msgClass.label());
	return s;
}
//_____________________________________________ Timer

Timer::Timer(Uid key, bool active, bool periodic, uint64_t interval)
	: Uid(key), _active(active), _periodic(periodic), _interval(interval) {
	load();
}

bool Timer::active() { return _active; }

void Timer::active(bool t) { _active = t; }

Uid Timer::key() { return id(); }

void Timer::load() { _expiresAt = Sys::millis() + _interval; }

void Timer::reload() {
	if (_periodic) {
		load();
	} else {
		_active = false;
	}
}

bool Timer::expired() { return Sys::millis() > _expiresAt; }

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

TimerScheduler::TimerScheduler() {}

Timer* TimerScheduler::find(Uid key) {
	for (Timer* timer : _timers) {
		if (timer->key() == key)
			return timer;
	}
	return 0;
}

Timer* TimerScheduler::findNextTimeout() {
	uint64_t nextTimeout = UINT64_MAX;
	Timer* to = 0;
	for (auto t : _timers) {
		if (t->active() && t->expiresAt() < nextTimeout) {
			to = t;
			nextTimeout = t->expiresAt();
		}
	}
	return to;
}

Uid TimerScheduler::startPeriodicTimer(Uid key, MsgClass cls, uint32_t msec) {
	Timer* timer = find(key.id());

	if (timer == 0) {
		_timers.push_back(new Timer(key, true, true, msec));
	} else {
		timer->set(true, true, msec);
	}
	return key.id();
}

Uid TimerScheduler::startSingleTimer(Uid key, MsgClass, uint32_t msec) {
	Timer* timer = find(key.id());

	if (timer == 0) {
		_timers.push_back(new Timer(key, true, false, msec));
	} else {
		timer->set(true, false, msec);
	}
	return key.id();
}

void TimerScheduler::cancel(Uid key) {
	Timer* timer = find(key);
	if (timer)
		timer->active(false);
	else
		WARN(" timer.cancel()  key not found : %s ", key.label());
}

void TimerScheduler::cancelAll() {
	for (Timer* t : _timers) {
		t->active(false);
	}
}

bool TimerScheduler::isTimerActive(Uid key) {
	for (Timer* t : _timers) {
		if (t->key() == key) {
			return t->active();
		}
	}
	WARN(" timer.isTimerActive()  key not found : %s ", key.label());
	return false;
}

//___________________________________________________________________ Receive
//

Receive::Receive() {}

Receive& Receive::match(MsgClass msgClass, MessageHandler doSome) {
	_receivers.push_back(new Receiver(msgClass, Receiver::alwaysTrue, doSome));
	return *this;
}

Receive& Receive::build() { return *this; }

void Receive::onMessage(Envelope& envelope) {
	bool found=false;
//	WARN(" onMessage '%s'=>'%s'=>'%s'",envelope.sender->label(),envelope.msgClass.label(),envelope.receiver->label());

	for (auto receiver : _receivers) {
		if (receiver->match(envelope)) {
//			envelope.offset(0);
			receiver->onMessage(envelope);
			found=true;
		}
	}
	if ( !found ) {
		WARN(" no receiver : '%s'=>'%s'=>'%s'",envelope.sender->label(),envelope.msgClass.label(),envelope.receiver->label());
	}
}

//______________________________________________________________ ActorCell
//

list<ActorCell*> ActorCell::_actorCells;

ActorCell::ActorCell(ActorSystem& system, ActorRef& ref, Mailbox& mailbox,
                     MessageDispatcher& dispatcher)
	: _mailbox(mailbox), _system(system), _dispatcher(dispatcher), _self(ref) {
	_currentMessage = 0;
	_timers = 0;
	_lastReceive = UINT64_MAX-2*UINT32_MAX;
	_inactivityPeriod = UINT32_MAX;
	_receive = 0;
	_prevReceive = 0;
	_actorCells.push_back(this);
}

const char* ActorCell::path() { return _self.path(); }

Mailbox& ActorCell::mailbox() { return _mailbox; }

ActorRef& ActorCell::self() { return _self; }

uint64_t ActorCell::expiresAt() { return _lastReceive + _inactivityPeriod; }

MessageDispatcher& ActorCell::dispatcher() { return _dispatcher; }

ActorCell* ActorCell::lookup(ActorRef* ref) { return ref->cell(); }

void ActorCell::actor(Actor* actor) { _actor=actor;};

void ActorCell::invoke(Envelope& msg) {
	_currentMessage = &msg;
	_receive->onMessage(msg);
}

ActorRef& ActorCell::sender() { return *_currentMessage->sender; }

ActorSystem& ActorCell::system() { return _system; }

void ActorCell::become(Receive& r, bool discardOld) {
	if (!discardOld) {
		_prevReceive = &r;
	}
	_receive = &r;
}

void ActorCell::unbecome() {
	if (_prevReceive != 0) {
		_receive = _prevReceive;
		_prevReceive = 0;
	} else
		_receive = &NullReceive;
}

void ActorCell::setReceiveTimeout(uint32_t msec) {
	_inactivityPeriod = msec;
	_lastReceive = Sys::millis();
}

uint32_t ActorCell::receiveTimeout() { return _inactivityPeriod; }

void ActorCell::resetReceiveTimeout() { _lastReceive = Sys::millis(); }

bool ActorCell::hasReceiveTimedOut() {
	return Sys::millis() > (_lastReceive + _inactivityPeriod);
}

TimerScheduler& ActorCell::timers() {
	if (_timers == 0)
		_timers = new TimerScheduler();
	return *_timers;
}

bool ActorCell::hasTimers() { return _timers != 0; }

list<ActorCell*>& ActorCell::actorCells() { return _actorCells; }

//________________________________________________ MessageDispatcher
//

MessageDispatcher::MessageDispatcher()
	: _txdEnvelope(*new Envelope(256)), _rxdEnvelope(*new Envelope(256)) {
	_unhandledCell = 0;
}

void MessageDispatcher::attach(Mailbox& mailbox) {
	_mailboxes.push_back(&mailbox);
}

void MessageDispatcher::attach(ActorCell& cell) {
	_actorCells.push_back(&cell);
}

void MessageDispatcher::unhandled(ActorCell* cell) { _unhandledCell = cell; }

Envelope& MessageDispatcher::txdEnvelope() { return _txdEnvelope; }

void MessageDispatcher::nextWakeup(uint64_t t) {
	//   INFO(" %lld ", t);
	if (_nextWakeup > t)
		_nextWakeup = t;
}

uint64_t MessageDispatcher::nextWakeup() { return _nextWakeup; }

void MessageDispatcher::execute() {
	bool busy = true;
	std::string s;
	uint32_t loopCount = 0;
	_nextWakeup = Sys::millis() + 10001;
	while (busy) {
		busy = false;
		for (auto mb : _mailboxes) {
			// take a message from each mailbox if needed
			if (mb->hasMessages()) {
				mb->dequeue(_rxdEnvelope); // load envelope and payload
				//			INFO("%s",_rxdEnvelope.toString().c_str());
				/*				INFO(" %s : %s => %s => %s "
								     ,mb->name()
								     ,_rxdEnvelope.sender->label()
								     ,_rxdEnvelope.msgClass.label()
								     ,_rxdEnvelope.receiver->label());*/
				assert(_rxdEnvelope.receiver != 0);
				assert(_rxdEnvelope.sender != 0);
				ActorCell* cell = _rxdEnvelope.receiver->cell();
				if (cell) {
					cell->invoke(_rxdEnvelope);
					cell->resetReceiveTimeout();
				} else {
					if (_unhandledCell) {
						_unhandledCell->invoke(_rxdEnvelope);
					} else {
						WARN(" no Receive found  %s->%s:%s ",
						     _rxdEnvelope.sender->path(),
						     _rxdEnvelope.receiver->path(),
						     _rxdEnvelope.msgClass.label());
					}
				}
				busy = true;
			}
		};

		for (auto ac : _actorCells) {
			Timer* timer;
			if (ac->hasTimers() && (timer = ac->timers().findNextTimeout())) {
				if (timer->expiresAt() < Sys::millis()) {
					//                    INFO(" TimerExpired => %s",
					//                    ac->self().label());
					timer->reload(); // retrigger now message will be send
					Msg timerExpired(TimerExpired);
					timerExpired(UID_TIMER,timer->id());
					ac->self().tell(NoSender(), timerExpired);
					busy = true;
				} else {
					nextWakeup(timer->expiresAt());
				}
			}
			if (ac->hasReceiveTimedOut()) {
				Msg receiveTimeout(ReceiveTimeout); // TBD
				ac->self().tell(NoSender(), receiveTimeout);
				ac->resetReceiveTimeout();
				busy = true;
			} else {
				nextWakeup(ac->expiresAt());
			}
		};
		for (auto mb : _mailboxes) {
			if (mb->hasMessages()) {
				nextWakeup(0);
			}
		}
		if (loopCount++ > 1000)
			return;
	};
}
// void arduinoLoop() { defaultDispatcher.execute(); }
