#include <stdio.h>

//============================================================================
// Name        : akkaMicro.cpp
// Author      : Lieven
// Version     :
// Copyright   : Enjoy teh source
// Description : Akka alike framework in C++ for embedded systems : low RAM
//============================================================================

#include <iostream>
#include <stdint.h>
#include <stdarg.h>

using namespace std;

#include <Erc.h>
#include <Cbor.h>
#include <CborQueue.h>
#include <Str.h>
#include <LinkedList.hpp>
#include <Log.h>

//#define MAX_FMT_LEN 22

// ( Actor -> mailbox -> thread )
// all coRoutines == same mailbox, a single thread
//  ActorRef : shorthand uid_t reference
//


typedef uid_t MsgClass;
typedef void (*MsgHandler)(void);

class MessageQueue: public CborQueue
{
public:
	MessageQueue(uint32_t size);
};


class MailBox: public MessageQueue
{
public:
	MailBox(uint32_t size):MessageQueue(size) {};
	bool hasMessages();
	void awake();
	void run()
	{
		while (true) {
			while (hasMessages()) {

			}
		}
	}
};

class Message;

class ActorRef
{
	uid_t _uid;
public:

	void ask(ActorRef dst, MsgClass type, Message& msg,uint32_t timeout);
	void forward(Message& msg);
	void tell(Message& message,ActorRef sender);
	void tell(ActorRef dst, MsgClass type, const char* format, ...); // queue message src,dst,msgtype,idx,...

};




//_____________________________________________________________________ Envelope
class Envelope
{
	ActorRef _sender;
	ActorRef _receiver;
	MsgClass _msgClass;
	uint32_t _id;
	Str _fmt(22);
public:
	ActorRef getSender();
	ActorRef getReceiver();
	MsgClass getMsgClass();
	uint32_t getId();
	const char *getFormat();
	bool read(Cbor& message)
	{
		return message.scanf("iiiiS", &_sender, &_receiver, &_msgClass, &_id,
		                     &Envelope::_fmt);
	}
};

//_____________________________________________________________________ Message
class Message: public Envelope
{
	Cbor& _message;

public:

	bool read(Cbor& message, const char* fmt, ...)
	{
		_message = message;
		if (Envelope::read(message)) {
			if (strcmp(fmt, getFormat()) == 0) {
				va_list args;
				va_start(args, fmt);
				bool b = _message.vscanf(fmt, args);
				va_end(args);
				return b;
			} else {
				ERROR(" incompatible format %s : %s ",fmt,getFormat());
			}
		}
		return false;
	}

	Message();
	Erc serialize(Cbor& bytes);
	Erc deSerialize(Cbor& bytes);
};
typedef bool (*MsgMatch)(Message& msg);

class Actor;
//_____________________________________________________________________ ActorSystem
class ActorSystem
{
	const char* _name;
	MessageQueue _queue;
	LinkedList<MailBox> mailboxes;

public:
	ActorSystem(const char* name, uint32_t queueSize) :
		_name(name), _queue(queueSize)
	{

	}
	Erc queue(Cbor& message);
	void registerActor(Actor&);

	void addActor(Actor&);
	Actor* findActor(ActorRef);
	void deleteActor(ActorRef);
	void callHandlers(ActorRef src, MsgClass type);
	void loop();

	void setReceiveTimeout(uint32_t msec);
	uint32_t getReceiveTimeout();
	void stop(ActorRef);
	template<class T> ActorRef actorOf(const char* name, ...)
	{
		T* t = new T(name);
		addActor(t);
		return t.self();
	}
};

class Context: public ActorSystem
{

};


/*
 * S : String
 * s : const char*
 * i : int
 * l : long
 * f : double
 * b : boolean
 *
 * iiiis... == sender,receiver,msgClass,id,format,...
 *
 * 	bytes.get("iiiis",&_sender,&_receiver,&_msgClass,&_id)
 *
 *
 */
//_____________________________________________________________________ Receive

class Receive
{
	// table of src,msgType,bool func, function(Message)
public:
	Receive& match(ActorRef dst, MsgClass msgClass, function f);
	Receive& build();
};
//_____________________________________________________________________ Actor
class Actor
{
	const char* _name;
	ActorSystem& _system;
	const ActorRef _self;

	// Linkedlist<MsgHandler> first;
public:

	Actor(ActorSystem& system, const char* name):_name(name),_system(system),_self(H(name))
	{
		system.addActor(*this);
	}
	~Actor()
	{
	}
	ActorRef getDestination();
	Erc getMessage(const char* fmt, ...);

	void event(MsgClass type, const char* fmt, ...); // queue message src,ANY_ACTOR,msgType,idx,...
	void ask(ActorRef dst,Message& msg);

	ActorRef getSender();
	virtual Receive& createReceive() =0;
	ActorSystem& getSystem();
	Context& getContext();
	Receive& receiveBuilder();
	virtual void preStart()
	{
	}
};

class Mqtt: public Actor
{
public:
	Mqtt(ActorSystem& system) :
		Actor(system,"mqtt")
	{
	}
	~Mqtt()
	{
	}
	void preStart()
	{
	}
	Receive& createReceive()
	{
		return receiveBuilder(); //.match(wifi,H("connected"),[]-> {}).build();
	}
};
class Wifi: public Actor
{
public:
	Wifi(ActorSystem& system) :
		Actor(system,"wifi")
	{
	}
	~Wifi()
	{
	}
	Receive& createReceive()
	{
		return receiveBuilder();
	}
};

ActorSystem sys("sys",20000);

int main()
{
	Wifi wifi(sys);
	Mqtt mqtt(sys);
	cout << "!!!Hello World!!!" << endl; // prints !!!Hello World!!!
	return 0;
}
