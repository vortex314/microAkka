//============================================================================
// Name        : akkaMicro.cpp
// Author      : Lieven
// Version     :
// Copyright   : Enjoy the source
// Description : Akka alike framework in C++ for embedded systems : low RAM
//============================================================================
#include "Akka.h"

//_____________________________________________ STATIC

// MsgClass& AnyClass;
// ActorMsgBus bus;
// Mailbox deadLetterMailbox("deadletterMailbox", 1, 100);
// ActorSystem defaultActorSystem("system");

// Envelope& NoMessage;
Receive& nullReceive;

typedef void (*MsgHandler)(void);

//_______________________________________________ UidType
//

UidType::UidType(const char* name) { _id = UID.hash(name); }

UidType::UidType(uint16_t id) : _id(id) {}

bool UidType::operator==(UidType v) { return _id == v._id; }

const char* UidType::label() { return UID.label(_id); }

uid_type UidType::id() { return _id; }

void UidType::id(uid_type id) { _id = id; }

//_________________________________________________ MsgClass

MsgClass& MsgClass::ReceiveTimeout() {
    static MsgClass ReceiveTimeout("ReceiveTimeout");
    return ReceiveTimeout;
}
MsgClass& MsgClass::TimerExpired() {
    static MsgClass TimerExpired("TimerExpired");
    return TimerExpired;
}
MsgClass& MsgClass::PoisonPill() {
    static MsgClass PoisonPill("PoisonPill");
    return PoisonPill;
}
MsgClass& MsgClass::AnyClass() {
    static MsgClass AnyClass("AnyClass");
    return AnyClass;
}

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

list<ActorRef*> ActorRef::_actorRefs;
ActorRef& ActorRef::NoSender() {
    static ActorRef NoSender("NoSender", 0);
    return NoSender;
}

ActorRef* ActorRef::lookup(uid_type id) {

    for (ActorRef* ref : _actorRefs)
        if (ref->id() == id)
            return ref;
    return 0;
}

ActorRef::ActorRef(UidType id, Mailbox* mailbox) : _id(id) {
    INFO(" created ActorRef %s", id.label());
    _actorRefs.push_back(this);
    _cell = 0;
    _mailbox = mailbox;
}

bool ActorRef::isLocal() { return _cell == 0; }

ActorRef::ActorRef() : _id(NoSender().id()) {}

const char* ActorRef::path() { return _id.label(); }

Mailbox& ActorRef::mailbox() { return *_mailbox; }
void ActorRef::mailbox(Mailbox& mb) { _mailbox = &mb; }
uid_type ActorRef::id() { return _id.id(); }

bool ActorRef::operator==(ActorRef& v) { return _id == v._id; }
//______________________________________________ LocalActorRef

// Envelope env(1024);
void ActorRef::tell(ActorRef src, MsgClass cls, const char* fmt, ...) {
    static Envelope env(1024);

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
    static Envelope env(1024);

    env.message.clear();
    va_list args;
    va_start(args, fmt);
    env.header(*this, src, cls, id);
    env.message.vaddf(fmt, args);
    va_end(args);
    tell(src, env);
}

void ActorRef::tell(ActorRef sender, Envelope& envelope) {
    if (_mailbox != 0)
        _mailbox->enqueue(envelope);
    else {
        WARN(" no mailbox attached to ActorRef %s ", _id.label());
    }
}

void ActorRef::forward(Envelope& envelope) {
    envelope.receiver = this;
    if (_mailbox != 0)
        _mailbox->enqueue(envelope);
    else {
        WARN(" no mailbox attached to ActorRef %s ", _id.label());
    }
}
void ActorRef::cell(ActorCell* cell) { _cell = cell; }

ActorCell* ActorRef::cell() { return _cell; }

//____________________________________________________________ ActorSelection

ActorSelection::ActorSelection(UidType id) : ActorRef(id, 0) {}

//____________________________________________________________ Envelope
//

Envelope::Envelope(uint32_t size)
    : sender(&ActorRef::NoSender()), receiver(&ActorRef::NoSender()),
      msgClass(MsgClass::AnyClass()), id(0), message(size) {}

Envelope::Envelope(ActorRef& snd, ActorRef& rcv, MsgClass clz)
    : sender(&snd), receiver(&rcv), msgClass(clz), id(newId()), message(32) {}

uid_type Envelope::idCounter = 0;

uint32_t Envelope::newId() { return idCounter++; }

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

string& Envelope::toString(string& s) {
    string_format(s, " dst : %s , src : %s , class : %s  ", receiver->path(),
                  sender->path(), msgClass.label());
    return s;
}

typedef bool (*MsgMatch)(Envelope& msg);

typedef std::function<void(Envelope&)> MessageHandler;
//________________________________________________________ MessageQueue
//
MessageQueue::MessageQueue(int queueSize, int messageSize)
    : _cborQueue(queueSize), _cbor(messageSize) {}

bool MessageQueue::hasMessages() { return _cborQueue.hasData(); }

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
    : UidType(name), _name(name), _defaultMailbox(&defaultMailbox),
      _defaultDispatcher(&defaultDispatcher),
      _defaultProps(defaultDispatcher, defaultMailbox) {}

UidType ActorSystem::uniqueId(const char* name) {
    string s, t;
    s += _name;
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

//___________________________________________________________ Receiver
//
Receiver::Receiver(MsgClass msgClass, MessageMatcher matcher,
                   MessageHandler handler)
    : _msgClass(msgClass), _matcher(matcher), _handler(handler) {}

Receiver::Receiver(MsgClass msgClass, MessageHandler handler)
    : _msgClass(msgClass), _matcher(alwaysTrue), _handler(handler) {}

inline void Receiver::onMessage(Envelope& msg) { _handler(msg); }
bool Receiver::match(Envelope& msg) {
    if (_msgClass == msg.msgClass || _msgClass == MsgClass::AnyClass())
        return (_matcher(msg));
    return false;
}

string& Receiver::tostringing(string& s) {
    string_format(s, " class : %s  ", _msgClass.label());
    return s;
}
//_____________________________________________ Timer

Timer::Timer(UidType key, bool active, bool periodic, uint64_t interval)
    : UidType(key), _active(active), _periodic(periodic), _interval(interval) {
    load();
}
bool Timer::active() { return _active; }
void Timer::active(bool t) { _active = t; }
uid_type Timer::key() { return id(); }
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

Timer* TimerScheduler::find(uid_type key) {
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

uid_type TimerScheduler::startPeriodicTimer(UidType key, MsgClass cls,
                                            uint32_t msec) {
    Timer* timer = find(key.id());

    if (timer == 0) {
        _timers.push_back(new Timer(key, true, true, msec));
    } else {
        timer->set(true, true, msec);
    }
    return key.id();
}

uid_type TimerScheduler::startSingleTimer(UidType key, MsgClass,
                                          uint32_t msec) {
    Timer* timer = find(key.id());

    if (timer == 0) {
        _timers.push_back(new Timer(key, true, false, msec));
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
    for (Timer* t : _timers) {
        t->active(false);
    }
}

bool TimerScheduler::isTimerActive(uid_type key) {
    for (Timer* t : _timers) {
        if (t->key() == key) {
            return t->active();
        }
    }
    WARN(" timer.isTimerActive()  key not found : %s ", UID.label(key));
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
    for (auto receiver : _receivers) {
        if (receiver->match(envelope)) {
            envelope.message.offset(0);
            receiver->onMessage(envelope);
        }
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
    _lastReceive = UINT64_MAX;
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
        _receive = &nullReceive;
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
    : _txdEnvelope(*new Envelope(1024)), _rxdEnvelope(*new Envelope(1024)) {
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
    if (_nextWakeup > t)
        _nextWakeup = t;
}

uint64_t MessageDispatcher::nextWakeup() { return _nextWakeup; }

bool doneSomething = false;

bool busy() { return doneSomething == true; }

void idle() { doneSomething = false; }

void working() { doneSomething = true; }

void MessageDispatcher::execute() {
    //	static Envelope rxdEnvelope(1024);
    working();
    while (busy()) {
        idle();
        for (auto mb : _mailboxes) {
            // take a message from each mailbox if needed
            if (mb->hasMessages()) {
                mb->dequeue(_rxdEnvelope); // load envelope and payload
                ActorCell* cell = ActorCell::lookup(_rxdEnvelope.receiver);
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
                working();
            }
        };
        _nextWakeup = UINT64_MAX;
        for (auto ac : _actorCells) {
            Timer* timer;
            if (ac->hasTimers() && (timer = ac->timers().findNextTimeout())) {
                if (timer->expiresAt() < Sys::millis()) {
                    timer->reload(); // retrigger now message will be send
                    Envelope timerExpired(ActorRef::NoSender(), ac->self(),
                                          MsgClass::TimerExpired());
                    timerExpired.message.add(timer->key());
                    ac->self().tell(ActorRef::NoSender(), timerExpired);
                    working();
                } else {
                    nextWakeup(timer->expiresAt());
                }
            }
            if (ac->hasReceiveTimedOut()) {
                Envelope receiveTimeout(ActorRef::NoSender(), ac->self(),
                                        MsgClass::ReceiveTimeout()); // TBD
                ac->self().tell(ActorRef::NoSender(), receiveTimeout);
                ac->resetReceiveTimeout();
                working();
            } else {
                nextWakeup(ac->expiresAt());
            }
        };
        for (auto mb : _mailboxes) {
            if (mb->hasMessages())
                nextWakeup(0);
        };
    }
}

// void arduinoLoop() { defaultDispatcher.execute(); }
