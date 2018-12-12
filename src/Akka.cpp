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

unordered_map<uid_type, void*>* Uid::_uids;
unordered_map<uid_type, void*>* Uid::uids() {
    if (_uids == 0) {
        _uids = new unordered_map<uid_type, void*>();
    }
    return _uids;
}

Uid::Uid(const char* label) { _id = add(label); }
Uid::Uid(uid_type id) { _id = id; }
Uid::Uid(void *pv) { _id = add(pv); }

uid_type Uid::hash(const char* s) { return H(s); }
uid_type Uid::add(const char* label) {
    uid_type h;
    char* l = new char[strlen(label) + 1]();
    h = hash(label);
    strcpy(l, label);
	if ( find(h)==0)
		uids()->emplace(h, (void*)l);
    return h;
}

const char* Uid::label(uid_type id) {
    std::unordered_map<uid_type, void*>::const_iterator p = uids()->find(id);
    if (p == uids()->end())
        return 0;
    return (const char*)(p->second);
}
const char* Uid::label() { return Uid::label(_id); }

#define PTR_LENGTH sizeof(void*)
#define UID_LENGTH sizeof(uid_type)
uid_type Uid::hash(void* pv) {
	uid_type h;
	union {
		void* ptr;
		uid_type uids[PTR_LENGTH/UID_LENGTH];
	};
	ptr = pv;
	h = uids[0];
	for(int i=1;i<sizeof(uids);i++){
		h ^= uids[i];
	}
	return h;
}
uid_type Uid::add(void* pv) {
    uid_type h = hash(pv);
	if ( object(h)==0)
    uids()->emplace(h, pv);
    return h;
}
void* Uid::object(uid_type d){
	std::unordered_map<uid_type, void*>::const_iterator p = uids()->find(id);
    if (p == uids()->end())
        return 0;
    return p->second;
}
void* Uid::object(){
	return object(_id);
}




// void* Uid::object(uid_type id);
// Envelope& NoMessage;
Receive& nullReceive;

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

unordered_map<uid_type, ActorRef*> ActorRef::_actorRefs;

ActorRef* ActorRef::lookup(Uid uid) {
    std::unordered_map<uid_type, ActorRef*>::const_iterator got =
        _actorRefs.find(uid.id());
    if (got != _actorRefs.end())
        return got->second;
    return 0;
}

ActorRef::ActorRef(Uid uid, Mailbox* mailbox) : Uid(uid) {
    INFO(" created ActorRef %s", uid.label());
    _actorRefs.emplace(uid.id(), this);
    _cell = 0;
    _mailbox = mailbox;
}

bool ActorRef::isLocal() { return _cell == 0; }

ActorRef::ActorRef() : Uid(NoSender) {}

const char* ActorRef::path() { return label(); }

Mailbox& ActorRef::mailbox() { return *_mailbox; }
void ActorRef::mailbox(Mailbox& mb) { _mailbox = &mb; }

bool ActorRef::operator==(ActorRef& v) { return id() == v.id(); }
//______________________________________________ LocalActorRef

void ActorRef::tell(ActorRef& src, MsgClass cls, const char* fmt, ...) {
    static Envelope env(256);

    env.message.clear();
    va_list args;
    va_start(args, fmt);
    env.header(*this, src, cls);
    env.message.vaddf(fmt, args);
    va_end(args);
    tell(src, env);
}

void ActorRef::tell(ActorRef& src, MsgClass cls, uint16_t id, const char* fmt,
                    ...) {
    static Envelope env(256);

    env.message.clear();
    va_list args;
    va_start(args, fmt);
    env.header(*this, src, cls, id);
    env.message.vaddf(fmt, args);
    va_end(args);
    tell(src, env);
}

void ActorRef::tell(ActorRef& sender, Envelope& envelope) {
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
    : sender(&NoSender), receiver(&NoSender), msgClass(AnyClass), id(0),
      message(size) {}

Envelope::Envelope(ActorRef& snd, ActorRef& rcv, MsgClass clz)
    : sender(&snd), receiver(&rcv), msgClass(clz), id(newId()), message(32) {}

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
    : _cborQueue(queueSize), _txd(messageSize), _rxd(messageSize) {}

bool MessageQueue::hasMessages() { return _cborQueue.hasData(); }

void MessageQueue::enqueue(Envelope& msg) {
    _txd.clear();
    _txd.add(msg.receiver->id());
    _txd.add(msg.sender->id());
    _txd.add(msg.msgClass.id());
    _txd.add(msg.id);
    _txd.append(msg.message);
    _cborQueue.put(_txd);
}

void MessageQueue::dequeue(Envelope& msg) {
    _cborQueue.get(_rxd);
    uid_type uid;
    _rxd.get(uid);
    msg.receiver = ActorRef::lookup(uid);
    _rxd.get(uid);
    msg.sender = ActorRef::lookup(uid);
    _rxd.get(uid);
    msg.msgClass = MsgClass(uid);
    _rxd.get(msg.id);
    msg.message.clear();
    while (_rxd.hasData()) {
        msg.message.write(_rxd.read());
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
    : Uid(name), _name(name), _defaultMailbox(&defaultMailbox),
      _defaultDispatcher(&defaultDispatcher),
      _defaultProps(defaultDispatcher, defaultMailbox) {}

Uid ActorSystem::uniqueId(const char* name) {
    string s, t;
    s += _name;
    s += "/";
    s += name;
    uid_type hash = H(s.c_str());
    if (Uid::label(hash) == 0) {
        return Uid(name);
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
    bool busy=true;
    _nextWakeup = Sys::millis()+10001;
    while (busy) {
        busy=false;
        for (auto mb : _mailboxes) {
            // take a message from each mailbox if needed
            if (mb->hasMessages()) {
                mb->dequeue(_rxdEnvelope); // load envelope and payload
                ActorCell* cell = _rxdEnvelope.receiver.cell();
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
                busy=true;
            }
        };
    
    for (auto ac : _actorCells) {
        Timer* timer;
        if (ac->hasTimers() && (timer = ac->timers().findNextTimeout())) {
            if (timer->expiresAt() < Sys::millis()) {
                timer->reload(); // retrigger now message will be send
                Envelope timerExpired(NoSender, ac->self(), TimerExpired);
                timerExpired.message.add(timer);
                ac->self().tell(NoSender, timerExpired);
				busy=true;
            } else {
                nextWakeup(timer->expiresAt() ));
            }
        }
        if (ac->hasReceiveTimedOut()) {
            Envelope receiveTimeout(NoSender, ac->self(),
                                    ReceiveTimeout); // TBD
            ac->self().tell(NoSender, receiveTimeout);
            ac->resetReceiveTimeout();
            busy=true;
        } else {
            nextWakeup(ac->expiresAt() );
        }
    };
           for (auto mb : _mailboxes) {
               if (mb->hasMessages()) {
                   nextWakeup(0);
                   INFO("");
               }
           }
};

// void arduinoLoop() { defaultDispatcher.execute(); }
