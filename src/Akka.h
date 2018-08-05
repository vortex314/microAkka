/*
 * Akka.h
 *
 *  Created on: 2-aug.-2018
 *      Author: Lieven Merckx
 */

#ifndef SRC_AKKA_H_
#define SRC_AKKA_H_
#include <stdio.h>

//============================================================================
// Name        : akkaMicro.cpp
// Author      : Lieven
// Version     :
// Copyright   : Enjoy teh source
// Description : Akka alike framework in C++ for embedded systems : low RAM
//
// Mailbox 1 >-----> N Actor 1 <----> 1 ActorRef
//
// Actor 1 >---> 1 Receive 1 ----> N Receiver ----> ( filter, method )
//
// 1 mailbox == 1 Thread
// to avoid overhead of RTTI in embedded systems, a string identifier ( hashed )
// was used to interprete the message format
//
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

//#define MAX_FMT_LEN 22

// ( Actor -> mailbox -> thread )
// all coRoutines == same mailbox, a single thread
//  ActorRef : shorthand uid_t reference
//

typedef uid_t MsgClass;
typedef void (*MsgHandler)(void);

class Message;
class Receiver;
class Mailbox;

class ActorRef {

  public:
    uid_t uid;

    ActorRef();
    ActorRef(uid_type id);
    //    bool operator==(const ActorRef& dst) const;
    Mailbox* mailbox();
    void ask(ActorRef dst, MsgClass type, Message& msg, uint32_t timeout);
    void forward(Message& msg);
    void tell(Message& message, ActorRef sender);
    void tell(ActorRef dst, MsgClass type, const char* format, ...);
};

//_____________________________________________________________________ Message
class Message {
    static uint16_t idCounter;

  public:
    ActorRef sender;
    ActorRef receiver;
    MsgClass msgClass;
    uint32_t id;
    Cbor payload;
    Message(uint32_t size);
    Message& setHeader(ActorRef snd, ActorRef rcv, MsgClass clz);
    bool getHeader();
    static uint32_t newId();
    bool scanf(const char* fmt, ...);
    bool deserialize(Cbor& message);
    void serialize(Cbor& message);
    Str& toString(Str&);
};

//_____________________________________________________________________ Mailbox
class Mailbox {
    CborQueue cborQueue;
    LinkedList<Receiver*> receivers;
    static LinkedList<Mailbox*> mailboxes;

  public:
    Message rxdMessage;
    Message txdMessage;
    Mailbox(uint32_t queueSize, uint32_t messageSize);
    bool hasMessages();
    void enqueue(Message& msg);
    void dequeue(Message& msg);
    void handleMessage(Message& msg);
    void handleMessages();
    void addReceiver(Receiver* rcv);
    Str& toString(Str& str);
};

//_____________________________________________________________________ Message

typedef bool (*MsgMatch)(Message& msg);
typedef std::function<void(Message&)> MessageHandler;
typedef std::function<bool(Message&)> MessageMatcher;

class Actor;
//_____________________________________________________________________
// ActorSystem
class ActorSystem {
    const char* _name;

  public:
    Mailbox mailbox;
    ActorSystem(const char* name, uint32_t queueSize, uint32_t messageSize);
    Erc queue(Cbor& message);
    void registerActor(Actor&);
    ActorRef addActor(Actor& actor);
    Actor* findActor(ActorRef);
    void deleteActor(ActorRef);
    void callHandlers(ActorRef src, MsgClass type);
    void loop();

    void stop(ActorRef);
    template <class T> ActorRef actorOf(const char* name, ...) {
        T* actor = new T(name);
        addActor(*actor);
        actor->createReceive();
        return actor->self;
    }
};
//_____________________________________________________________________
// ActorContext
class ActorContext {
    uint64_t timeout;

  public:
    ActorContext();
    void cancelReceiveTimeout();
    void setReceiveTimeout(uint32_t msec);
};

//_____________________________________________________________________ Receiver
//

class Receiver {
    ActorRef _src;
    ActorRef _dst;
    MsgClass _msgClass;
    MessageMatcher _matcher;
    MessageHandler _handler;

  public:
    Receiver(ActorRef src, ActorRef dst, MsgClass msgClass,
             MessageMatcher matcher, MessageHandler handler);
    bool match(Message& msg);
    void handle(Message& msg);
    const static bool alwaysTrue(Message&) { return true; }
    Str& toString(Str& s);
};
//_____________________________________________________________________ Receive
//
class Receive {
    // table of src,msgType,bool func, function(Message)
    Actor& dst;
    //    LinkedList<Receiver*> receivers;

  public:
    Receive(Actor& dst);
    Receive& match(MsgClass msgClass, MessageHandler doSome);
    Receive& build();
};
//_____________________________________________________________________ Timer
//
class Timer {
    uid_type key;
    bool active;
    bool periodic;
    uint64_t expiresAt;

  public:
    Timer(uid_type key, bool active, bool periodic, uint64_t interval);
};

class TimerScheduler {
    void startPeriodicTimer(uid_type key, Message& msg, uint32_t msec);
    void startSingleTimer(uid_type key, Message& msg, uint32_t msec);
    void cancel(uid_type key);
    void cancelAll();
    bool isTimerActive(uid_type key);
};
//_____________________________________________________________________ Actor
class Actor {

    // Linkedlist<MsgHandler> first;
    static LinkedList<Actor*> actors;

  public:
    const char* name;
    ActorSystem* system;
    Mailbox* _mailbox;
    ActorRef self;
    Receive receive;

    static Mailbox* mailbox(ActorRef);

    const ActorContext context;

    Actor(const char* name);
    virtual ~Actor();
    ActorRef getDestination();

    void event(MsgClass type, const char* fmt,
               ...); // queue message src,ANY_ACTOR,msgType,idx,...
    void ask(ActorRef dst, Message& msg);
    // 	AbstractActor
    //------------------
    void setReceiveTimeout(uint32_t msec);
    uint32_t getReceiveTimeout();
    ActorSystem& getSystem();

    ActorRef getSelf();
    ActorRef getSender();
    const char* getName();
    virtual Receive& createReceive() = 0;
    Receive& receiveBuilder();
    virtual void preStart();
    virtual void postStop();
    virtual void unhandled(Message& msg);
    Timer getTimers();
};

class Dispatcher {
  public:
};

extern ActorRef AnyActor;
extern uid_type AnyClass;
extern ActorSystem actorSystem;

#endif /* SRC_AKKA_H_ */
