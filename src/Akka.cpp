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
#include <Log.h>
#include <Str.h>
#include <Uid.h>

//_____________________________________________- STATIC

MsgClass AnyClass("$ANY");
Receive Receive::nullReceive;
ActorMsgBus bus;
Mailbox deadLetterMailbox("deadletterMailbox", 1, 100);
ActorSystem defaultActorSystem("system");
MsgClass ReceiveTimeout("ReceiveTimeout");
MsgClass TimerExpired("TimerExpired");
ActorRef NoSender("NoSender");
ActorRef AnyActor("AnyActor");

typedef void (*MsgHandler)(void);

UidType::UidType(const char* name) { _id = UID.hash(name); }

UidType::UidType(uint16_t id) : _id(id) {}

bool UidType::operator==(UidType v) { return _id == v._id; }

const char* UidType::label() { return UID.label(_id); }

bool UidType::hasLabel() { return UID.find(_id) != 0; }

//_____________________________________________________________________
// Actor

Actor::~Actor() {}

LinkedList<AbstractActor*> AbstractActor::actors;

AbstractActor::AbstractActor() : _context(0) {}

AbstractActor::~AbstractActor() {}

void AbstractActor::context(ActorContext* ctx) { _context = ctx; }

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

void AbstractActor::preStart() {}

void AbstractActor::postStop() {}

void AbstractActor::unhandled(Envelope& msg) {
    INFO("unhandled message for Actor : %s ", self().path());
}

ActorRef AbstractActor::sender() {
    return context().mailbox().rxdEnvelope.sender;
}

TimerScheduler& AbstractActor::timers() { return context().timers(); }

void AbstractActor::withDispatcher(MessageDispatcher& dispatcher) {
    dispatcher.attach(context());
}

//____________________________________________________________ ActorRef

ActorRef::ActorRef(UidType id) : _id(id) {}

ActorRef::ActorRef() : _id(NoSender.id()) {}

bool ActorRef::operator==(ActorRef& dst) { return (_id == dst._id); }

void ActorRef::id(UidType id) { _id = id; }

UidType ActorRef::id() { return _id; }

const char* ActorRef::path() { return _id.label(); }

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
    Mailbox& mb = mailbox();

    mb.txdEnvelope.setHeader(msg.sender, *this, msg.msgClass);
    mb.txdEnvelope.message.append(msg.message);
    mb.enqueue(mb.txdEnvelope);
}

uid_type Envelope::idCounter = 0;

uint32_t Envelope::newId() { return idCounter++; }

//______________________________________________________________________ Message
//

Envelope::Envelope(uint32_t size)
    : sender(AnyActor), receiver(AnyActor), msgClass(AnyClass), id(0),
      message(size) {}

Envelope::Envelope(ActorRef snd, ActorRef rcv, MsgClass clz)
    : sender(snd), receiver(rcv), msgClass(clz), id(newId()), message(16) {
    message.addf("2222", snd.id(), rcv.id(), clz, id);
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

LinkedList<Mailbox*> Mailbox::_mailboxes;

Mailbox::Mailbox(const char* name, uint32_t queueSize, uint32_t messageSize)
    : _name(name), _cborQueue(queueSize), rxdEnvelope(messageSize),
      txdEnvelope(messageSize) {
    _mailboxes.add(this);
}

void Mailbox::enqueue(Envelope& msg) { _cborQueue.put(msg.message); }

void Mailbox::dequeue(Envelope& msg) { _cborQueue.get(msg.message); }

bool Mailbox::hasMessages() { return _cborQueue.hasData(); }

void Mailbox::handleMessages() {
    while (hasMessages()) {
        dequeue(rxdEnvelope); // load envelope and payload
        rxdEnvelope.getHeader();
        ActorContext* context = ActorContext::context(rxdEnvelope.receiver);
        if (context && context->mailbox() == *this) {
            context->receive().handle(rxdEnvelope);
        } else {
            WARN(" unknown destination ref %d ! trying remote. ",
                 rxdEnvelope.receiver.id());
        }
    }
    // TODO handle timeouts
}

LinkedList<Mailbox*>& Mailbox::mailboxes() { return _mailboxes; }
//_____________________________________________________________________
// ActorSystem
//
ActorSystem::ActorSystem(const char* name)
    : _name(name), _defaultMailbox(defaultMailbox),
      _deadLetterMailbox(deadLetterMailbox),_defaultDispatcher(defaultDispatcher) {}

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

inline void Receiver::handle(Envelope& msg) { _handler(msg); }
bool Receiver::match(Envelope& msg) {
    if (_msgClass == msg.msgClass || _msgClass == AnyClass)
        return (_matcher(msg));
    return false;
}

Str& Receiver::toString(Str& s) {
    return s.format(" class : %s  ", _msgClass.name());
}
//_____________________________________________ Timer

Timer::Timer(UidType key, bool active, bool periodic, uint64_t interval)
    : _key(key), _active(active), _periodic(periodic), _interval(interval) {
    load();
}
bool Timer::operator==(Timer& other) { return _key == other._key; }
bool Timer::active() { return _active; }
void Timer::active(bool t) { _active = t; }
UidType Timer::key() { return _key; }
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

void Receive::handle(Envelope& envelope) {
    _receivers.forEach([&envelope](Receiver* receiver) {
        if (receiver->match(envelope)) {
            receiver->handle(envelope);
        }
    });
}

//______________________________________________________________ ActorCell
//

ActorCell::ActorCell(UidType id, Mailbox& mailbox, ActorRef& self)
    : _id(id), _mailbox(mailbox), _self(self) {}
UidType ActorCell::id() { return _id; }

const char* ActorCell::path() { return _id.label(); }

Mailbox& ActorCell::mailbox() { return _mailbox; }

void ActorCell::mailbox(Mailbox& mailbx) { _mailbox = mailbx; }

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
                           ActorSystem& system, Mailbox& mailbox,
                           Receive& receive)
    : ActorCell(id, mailbox, self), _actor(actor), _system(system),
      _receive(receive) {
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

Receive& ActorContext::receive() { return _receive; }

void ActorContext::receive(Receive& r) { _receive = r; }

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
                            msg.msgClass.name());
                   })
            .build();
    }
} deadLetterActor;

MessageDispatcher::MessageDispatcher(){};

void MessageDispatcher::attach(Mailbox& mailbox) { _mailboxes.add(&mailbox); }
void MessageDispatcher::attach(ActorCell& cell) { _actorCells.add(&cell); }
void MessageDispatcher::execute() {
    Mailbox* mailbox;
    ActorContext* actorContext;
    Timer* timer;

    while (true) {
        mailbox =
            _mailboxes.findFirst([](Mailbox* mb) { return mb->hasMessages(); });
        if (mailbox)
            mailbox->handleMessages();
        else
            break;
    }
    while (true) {
        actorContext =
            ActorContext::actorContexts().findFirst([&timer](ActorContext* ac) {
                if (ac->hasTimers() &&
                    (timer = ac->timers().findNextTimeout()) &&
                    (timer->expiresAt() < Sys::millis())) {
                    timer->reload(); // retrigger now message will be send
                    return true;
                };
                return false;
            });
        if (actorContext) {
            Envelope timerExpired(NoSender, actorContext->self(), TimerExpired);
            timerExpired.message.addf("2", timer->key());
            actorContext->self().tell(NoSender, timerExpired);
        };
    }
}

void loop() {
    Mailbox* mailbox;
    ActorContext* actorContext;
    Timer* timer;

    mailbox = Mailbox::mailboxes().findFirst(
        [](Mailbox* mb) { return mb->hasMessages(); });
    if (mailbox)
        mailbox->handleMessages();

    actorContext =
        ActorContext::actorContexts().findFirst([&timer](ActorContext* ac) {
            if (ac->hasTimers() && (timer = ac->timers().findNextTimeout()) &&
                (timer->expiresAt() < Sys::millis())) {
                timer->reload(); // retrigger now message will be send
                return true;
            };
            return false;
        });
    if (actorContext) {
        Envelope timerExpired(NoSender, actorContext->self(), TimerExpired);
        timerExpired.message.addf("2", timer->key());
        actorContext->self().tell(NoSender, timerExpired);
    };
}
