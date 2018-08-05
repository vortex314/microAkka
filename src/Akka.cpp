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
uid_type AnyClass = 0;

typedef uid_t MsgClass;
typedef void (*MsgHandler)(void);

ActorRef::ActorRef() {}

ActorRef::ActorRef(uid_type id) { uid = id; }
void ActorRef::ask(ActorRef dst, MsgClass type, Message& msg,
                   uint32_t timeout) {}

void ActorRef::forward(Message& msg) {}

void ActorRef::tell(ActorRef src, MsgClass cls, const char* fmt, ...) {

    Mailbox* mailbox = Actor::mailbox(*this);
    if (mailbox == 0)
        return;
    va_list args;
    va_start(args, fmt);
    mailbox->txdMessage.setHeader(src, uid, cls).payload.vaddf(fmt, args);
    va_end(args);

    mailbox->enqueue(mailbox->txdMessage);
}

bool operator==(const ActorRef& src, const ActorRef& dst) {
    return (dst.uid == src.uid);
}

uint16_t Message::idCounter = 0;

uint32_t Message::newId() { return idCounter++; }

//______________________________________________________________________ Message
//

Message::Message(uint32_t size) : payload(size) {}

Message& Message::setHeader(ActorRef snd, ActorRef rcv, MsgClass clz) {
    payload.offset(0);
    sender = snd;
    receiver = rcv;
    msgClass = clz;
    id = newId();
    payload.addf("2222", snd.uid, rcv.uid, clz, id);
    return *this;
}

bool Message::getHeader() {
    return payload.scanf("2222", &sender.uid, &receiver.uid, &msgClass, &id);
}

bool Message::scanf(const char* fmt, ...) {
    bool b = false;
    va_list args;
    va_start(args, fmt);
    b = payload.vscanf(fmt, args);
    va_end(args);
    return b;
}

Str& Message::toString(Str& s) {
    return s.format(" dst : %s , src : %s , class : %s  ",
                    UID.label(receiver.uid), UID.label(sender.uid),
                    UID.label(msgClass));
}

typedef bool (*MsgMatch)(Message& msg);

typedef std::function<void(Message&)> MessageHandler;

//_____________________________________________________________________ Mailbox

LinkedList<Mailbox*> mailboxes();

Mailbox::Mailbox(uint32_t queueSize, uint32_t messageSize)
    : cborQueue(queueSize), rxdMessage(messageSize), txdMessage(messageSize) {}

void Mailbox::enqueue(Message& msg) { cborQueue.put(msg.payload); }

void Mailbox::dequeue(Message& msg) {
    cborQueue.get(msg.payload);
    msg.getHeader();
}

bool Mailbox::hasMessages() { return cborQueue.hasData(); }

void Mailbox::addReceiver(Receiver* rcv) { receivers.add(rcv); };
//_____________________________________________________________________
// ActorSystem
//
ActorSystem::ActorSystem(const char* name, uint32_t queueSize,
                         uint32_t messageSize)
    : _name(name), mailbox(queueSize, messageSize) {}

ActorRef ActorSystem::addActor(Actor& actor) {
    actor.system = this;
    actor._mailbox = &mailbox; // point to default mailbox
    return actor.self;
}

//_____________________________________________________________________ Receiver
//
Receiver::Receiver(ActorRef src, ActorRef dst, MsgClass msgClass,
                   MessageMatcher matcher, MessageHandler handler)
    : _src(src), _dst(dst), _msgClass(msgClass), _matcher(matcher),
      _handler(handler) {}

void Receiver::handle(Message& msg) { _handler(msg); }
bool Receiver::match(Message& msg) {
    if (_dst == msg.receiver || _dst == AnyActor) {
        if (_msgClass == msg.msgClass || _msgClass == AnyClass) {
            if (_src == msg.sender || _src == AnyActor) {
                return true;
            }
        }
    }
    return false;
}

Str& Receiver::toString(Str& s) {
    return s.format(" dst : %s , src : %s , class : %s  ", UID.label(_dst.uid),
                    UID.label(_src.uid), UID.label(_msgClass));
}

//_____________________________________________________________________
// Receive
//
Receive::Receive(Actor& d) : dst(d) {}

Receive& Receive::match(MsgClass msgClass, MessageHandler doSome) {
    Receiver* receiver = new Receiver(AnyActor, dst.self, msgClass,
                                      Receiver::alwaysTrue, doSome);
    dst._mailbox->addReceiver(receiver);
    return *this;
}

Receive& Receive::build() { return *this; }

//_____________________________________________________________________
// Receive

//_____________________________________________________________________
// ActorContext
//
ActorContext::ActorContext() {}

//_____________________________________________________________________
// Actor

LinkedList<Actor*> Actor::actors;

Actor::Actor(const char* name) : name(name), self(H(name)), receive(*this) {
    actors.add(this);
}

Actor::~Actor() {}

Receive& Actor::receiveBuilder() { return receive; }

void Actor::preStart() {}

void Actor::postStop() {}

void Actor::unhandled(Message& msg) {
    INFO("unhandled message for Actor : %s ", name);
}

void Actor::setReceiveTimeout(uint32_t msec) {}

uint32_t Actor::getReceiveTimeout() { return 0; }

ActorRef Actor::getSender() { return _mailbox->rxdMessage.sender; }

Mailbox* Actor::mailbox(ActorRef ref) {
    Actor* actor =
        actors.findFirst([ref](Actor* actor) { return (actor->self == ref); });
    if (actor)
        return actor->_mailbox;
    return 0;
}

//________________________________________________________________________________

void ActorSystem::stop(ActorRef) {}

void Mailbox::handleMessage(Message& msg) {
    receivers.forEach([&msg](Receiver* rcv) {
        if (rcv->match(msg))
            rcv->handle(msg);
    });
}

void Mailbox::handleMessages() {
    while (hasMessages()) {
        dequeue(rxdMessage); // load envelope and payload
        handleMessage(rxdMessage);
    }
}