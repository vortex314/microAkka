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

#include <functional>
#include <stdarg.h>
#include <stdint.h>

using namespace std;

#include <Cbor.h>
#include <CborQueue.h>
#include <LinkedList.hpp>

//_____________________________________________- STATIC

MsgClass AnyClass("$ANY");
ActorMsgBus bus;
Mailbox deadLetterMailbox("deadletterMailbox", 1, 100);
ActorSystem defaultActorSystem("system");
MsgClass ReceiveTimeout("ReceiveTimeout");
MsgClass TimerExpired("TimerExpired");
ActorRef NoSender("NoSender");
ActorRef AnyActor("AnyActor");
Envelope NoMessage(NoSender, NoSender, "");

typedef void (*MsgHandler)(void);

UidType::UidType(const char* name) {
    _id = UID.hash(name);
    _name = label();
}

UidType::UidType(uint16_t id) : _id(id) { _name = label(); }

bool UidType::operator==(UidType v) { return _id == v._id; }

const char* UidType::label() { return UID.label(_id); }

bool UidType::hasLabel() { return UID.find(_id) != 0; }

uid_type UidType::id() { return _id; }

void UidType::id(uid_type id) {
    _id = id;
    _name = label();
}

//_____________________________________________________________________
// Actor

Actor::~Actor() {}

LinkedList<AbstractActor*> AbstractActor::actors;

AbstractActor::AbstractActor() : _context(0) {}

AbstractActor::~AbstractActor() {}

void AbstractActor::context(ActorContext* ctx) { _context = ctx; }

ActorRef AbstractActor::self() { return _context->self(); }

Receive& AbstractActor::receiveBuilder() { return *(new Receive()); }

ActorContext& AbstractActor::context() { return *_context; }

void AbstractActor::preStart() {}

void AbstractActor::postStop() {}

void AbstractActor::unhandled(Envelope& msg) {
    INFO("unhandled message for Actor : %s ", self().path());
}

ActorRef AbstractActor::sender() { return context().sender(); }

TimerScheduler& AbstractActor::timers() { return context().timers(); }

void AbstractActor::withDispatcher(MessageDispatcher& dispatcher) {
    dispatcher.attach(context());
}

//____________________________________________________________ ActorRef

ActorRef::ActorRef(UidType id) : UidType(id) {}

ActorRef::ActorRef() : UidType(NoSender.id()) {}

const char* ActorRef::path() { return label(); }

Mailbox& ActorRef::mailbox() {
    ActorContext* context = ActorContext::context(*this);
    if (context)
        return context->mailbox();
    else
        return remoteMailbox; // no local context found
}

/*
 void ActorRef::ask(ActorRef dst, MsgClass type, Envelope&
 msg,!array.is<char*>(0) uint32_t timeout) {
 }
 */
void ActorRef::tell(ActorRef src, MsgClass cls, const char* fmt, ...) {
    Envelope env(1024);
    Mailbox& mb = mailbox();
    va_list args;
    va_start(args, fmt);
    env.header(*this, src, cls);
    env.message.vaddf(fmt, args);
    va_end(args);

    mb.enqueue(env);
}

void ActorRef::tell(ActorRef sender, Envelope& envelope) {
    mailbox().enqueue(envelope);
}

void ActorRef::forward(Envelope& msg) {
    Mailbox& mb = mailbox();
    msg.receiver = *this;
    mb.enqueue(msg);
}

//______________________________________________________________________
// Envelope
//

Envelope::Envelope(uint32_t size)
    : sender(AnyActor), receiver(AnyActor), msgClass(AnyClass), id(0),
      message(size) {}

Envelope::Envelope(ActorRef snd, ActorRef rcv, MsgClass clz)
    : sender(snd), receiver(rcv), msgClass(clz), id(newId()), message(32) {}

uid_type Envelope::idCounter = 0;

uint32_t Envelope::newId() { return idCounter++; }

Envelope& Envelope::header(ActorRef rcv, ActorRef snd, MsgClass clz) {
    sender = snd;
    receiver = rcv;
    msgClass = clz;
    id = newId();
    return *this;
}

Envelope& Envelope::header(ActorRef rcv, ActorRef snd, MsgClass clz,
                           uint16_t i) {
    sender = snd;
    receiver = rcv;
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
    return s.format(" dst : %s , src : %s , class : %s  ", receiver.path(),
                    sender.path(), msgClass.label());
}

typedef bool (*MsgMatch)(Envelope& msg);

typedef std::function<void(Envelope&)> MessageHandler;
//____________________________________________________________ MessageQueue

MessageQueue::MessageQueue(int queueSize, int messageSize)
    : _cborQueue(queueSize), _cbor(messageSize) {}

bool MessageQueue::hasMessages() { return _cborQueue.hasData(); }

void MessageQueue::enqueue(Envelope& msg) {
    _cbor.clear();
    _cbor.add(msg.receiver.id());
    _cbor.add(msg.sender.id());
    _cbor.add(msg.msgClass.id());
    _cbor.add(msg.id);
    _cbor.append(msg.message);
    _cborQueue.put(_cbor);
}

void MessageQueue::dequeue(Envelope& msg) {
    _cborQueue.get(_cbor);
    uid_type uid;
    _cbor.get(uid);
    msg.receiver.id(uid);
    _cbor.get(uid);
    msg.sender.id(uid);
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

Mailbox::Mailbox(const char* name, uint32_t queueSize, uint32_t messageSize)
    : MessageQueue(queueSize, messageSize), _name(name) {
    _mailboxes.add(this);
}

LinkedList<Mailbox*>& Mailbox::mailboxes() { return _mailboxes; }
//_____________________________________________________________________
// ActorSystem
//
ActorSystem::ActorSystem(const char* name)
    : UidType(name), _defaultMailbox(&defaultMailbox),
      _deadLetterMailbox(&deadLetterMailbox),
      _defaultDispatcher(&defaultDispatcher) {}

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

Str& Receiver::toString(Str& s) {
    return s.format(" class : %s  ", _msgClass.label());
}
//_____________________________________________ Timer

Timer::Timer(UidType key, bool active, bool periodic, uint64_t interval)
    : UidType(key), _active(active), _periodic(periodic), _interval(interval) {
    load();
}
bool Timer::active() { return _active; }
void Timer::active(bool t) { _active = t; }
UidType Timer::key() { return label(); }
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

Timer* TimerScheduler::find(UidType key) {
    return _timers.findFirst([key](Timer* t) { return t->key() == key; });
}

Timer* TimerScheduler::findNextTimeout() {
    uint64_t nextTimeout = UINT64_MAX;
    Timer* t = 0;
    _timers.forEach([&nextTimeout, &t](Timer* timer) {
        if (timer->active() && timer->expiresAt() < nextTimeout) {
            t = timer;
            nextTimeout = timer->expiresAt();
        }
    });
    return t;
}

void TimerScheduler::startPeriodicTimer(UidType key, MsgClass cls,
                                        uint32_t msec) {
    Timer* timer = find(key);

    if (timer == 0) {
        _timers.add(new Timer(key, true, true, msec));
    } else {
        timer->set(true, true, msec);
    }
}
void TimerScheduler::startSingleTimer(UidType key, MsgClass, uint32_t msec) {
    Timer* timer = find(key);

    if (timer == 0) {
        _timers.add(new Timer(key, true, false, msec));
    } else {
        timer->set(true, false, msec);
    }
}
void TimerScheduler::cancel(UidType key) {
    Timer* timer = find(key);
    if (timer)
        timer->active(false);
    else
        WARN(" timer.cancel()  key not found : %s ", key.label());
}
void TimerScheduler::cancelAll() {
    _timers.forEach([](Timer* t) { t->active(false); });
}
bool TimerScheduler::isTimerActive(UidType key) {
    Timer* timer =
        _timers.findFirst([key](Timer* t) { return t->key() == key; });
    if (timer)
        return timer->active();
    else {
        WARN(" timer.isTimerActive()  key not found : %s ", key.label());
        return false;
    }
}

//_____________________________________________________________________
// Receive
//
Receive::Receive() {}

Receive& Receive::match(MsgClass msgClass, MessageHandler doSome) {
    Receiver* receiver = new Receiver(msgClass, Receiver::alwaysTrue, doSome);
    _receivers.add(receiver);
    return *this;
}

Receive& Receive::build() { return *this; }

void Receive::onMessage(Envelope& envelope) {
    _receivers.forEach([&](Receiver* receiver) {
        if (receiver->match(envelope)) {
            envelope.message.offset(0);
            receiver->onMessage(envelope);
        }
    });
}

//______________________________________________________________ ActorCell
//

ActorCell::ActorCell(UidType id, Mailbox& mailbox, ActorRef& self)
    : UidType(id), _mailbox(&mailbox), _self(self) {}

const char* ActorCell::path() { return label(); }

Mailbox& ActorCell::mailbox() { return *_mailbox; }

void ActorCell::mailbox(Mailbox& mailbx) { _mailbox = &mailbx; }

ActorRef ActorCell::self() { return _self; }

void ActorCell::self(ActorRef& ref) {
    ASSERT(ref.id() == _self.id());
    _self = ref;
}
//_________________________________________________________________________
// ActorContext
//
LinkedList<ActorContext*> ActorContext::_actorContexts;

ActorContext::ActorContext(UidType id, ActorRef self, AbstractActor& actor,
                           ActorSystem& system, Mailbox& mailbox)
    : ActorCell(id, mailbox, self), _actor(actor), _system(system), _receive(0),
      _currentMessage(&NoMessage) {
    _timers = 0;
    _receiveTimeout = UINT64_MAX;
    _inactivityPeriod = UINT32_MAX;
    _actorContexts.add(this);
}

ActorContext* ActorContext::context(ActorRef& ref) {
    return _actorContexts.findFirst(
        [&ref](ActorContext* ctx) { return ref.id() == ctx->id(); });
}

ActorContext& ActorContext::context(AbstractActor* actor) {
    return *_actorContexts.findFirst(
        [actor](ActorContext* ctx) { return &ctx->_actor == actor; });
}

void ActorContext::invoke(Envelope& msg) {
    _currentMessage = &msg;
    _receive->onMessage(msg);
}

ActorRef ActorContext::sender() { return _currentMessage->sender; }

Receive& ActorContext::receive() { return *_receive; }

ActorSystem& ActorContext::system() { return _system; }

void ActorContext::receive(Receive& r) { _receive = &r; }

TimerScheduler& ActorContext::timers() {
    if (_timers == 0)
        _timers = new TimerScheduler();
    return *_timers;
}

bool ActorContext::hasTimers() { return _timers != 0; }

LinkedList<ActorContext*>& ActorContext::actorContexts() {
    return _actorContexts;
}

//_____________________________________________

class DeadLetterActor : public AbstractActor {

  public:
    DeadLetterActor() {}
    ~DeadLetterActor() {}
    Receive& createReceive() {
        return receiveBuilder()
            .match(AnyClass,
                   [this](Envelope& msg) {
                       INFO(" DeadLetter Actor from '%s' to '%s' msg '%s' ",
                            msg.sender.path(), msg.receiver.path(),
                            msg.msgClass.label());
                   })
            .build();
    }
} deadLetterActor;

MessageDispatcher::MessageDispatcher() { _unhandledContext = 0; };

void MessageDispatcher::attach(Mailbox& mailbox) { _mailboxes.add(&mailbox); }
void MessageDispatcher::attach(ActorCell& cell) { _actorCells.add(&cell); }
void MessageDispatcher::unhandled(ActorContext* context) {
    _unhandledContext = context;
};

bool doneSomething = false;

bool busy() { return doneSomething == true; }

void idle() { doneSomething = false; }

void working() { doneSomething = true; }

void MessageDispatcher::execute() {
    static Envelope rxdEnvelope(1024);
    //    static Envelope timerExpired(NoSender, NoSender, TimerExpired)
    working();
    while (busy()) {
        idle();
        _mailboxes.forEach([this](Mailbox* mb) {
            if (mb->hasMessages()) {
                mb->dequeue(rxdEnvelope); // load envelope and payload
                ActorContext* context =
                    ActorContext::context(rxdEnvelope.receiver);
                if (context) {
                    context->invoke(rxdEnvelope);
                } else if (_unhandledContext) {
                    _unhandledContext->invoke(rxdEnvelope);
                } else {
                    INFO(" no Receive found  %s->%s:%s ",
                         rxdEnvelope.sender.path(), rxdEnvelope.receiver.path(),
                         rxdEnvelope.msgClass.label());
                }
                working();
            }
        });

        ActorContext::actorContexts().forEach([](ActorContext* ac) {
            Timer* timer;
            if (ac->hasTimers() && (timer = ac->timers().findNextTimeout()) &&
                (timer->expiresAt() < Sys::millis())) {
                timer->reload(); // retrigger now message will be send
                Envelope timerExpired(NoSender, ac->self(), TimerExpired);
                timerExpired.message.addf("i", timer->key());
                ac->self().tell(NoSender, timerExpired);
                working();
            }
        });
    }
}

void arduinoLoop() { defaultDispatcher.execute(); }
