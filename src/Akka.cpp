//============================================================================
// Name        : akkaMicro.cpp
// Author      : Lieven
// Version     :
// Copyright   : Enjoy the source
// Description : Akka alike framework in C++ for embedded systems : low RAM
//============================================================================
#include "Akka.h"

//_____________________________________________ STATIC


typedef void (*MsgHandler)(void);

//_______________________________________________ uid_type
//

//_________________________________________________ MsgClass


//_________________________________________________ Msg

Msg::Msg() : Xdr(12) {
//	INFO("ctor %X : [%d]",this,capacity());
	add(UD_CLS,(uid_type)0);
	add(UD_SRC,(uid_type)0);
	add(UD_DST,(uid_type)0);
}

Msg::Msg(uint32_t size) : Xdr(size) {
//	INFO("ctor %X : [%d]",this,capacity());
	add(UD_CLS,(uid_type)0);
	add(UD_SRC,(uid_type)0);
	add(UD_DST,(uid_type)0);
}

Msg::Msg(MsgClass cls) : Xdr(12) {
//	INFO("ctor %X : [%d]",this,capacity());

	add(UD_CLS,cls.id());
	add(UD_SRC,(uid_type)0);
	add(UD_DST,(uid_type)0);
}
Msg::Msg(Uid cls,Uid src) : Xdr(12) {
//	INFO("ctor %X : [%d]",this,capacity());
	add(UD_CLS,cls.id());
	add(UD_SRC,src.id());
	add(UD_DST,(uid_type)0);
}

Msg& Msg::clear() {
	Xdr::clear();
	add(UD_CLS,(uid_type)0);
	add(UD_SRC,(uid_type)0);
	add(UD_DST,(uid_type)0);
	return *this;
}

uint32_t Msg::id() {
	uint32_t id=0;
	get(UD_ID,id);
	return id;
}

Msg&  Msg::id(uint32_t i) {
	add(UD_ID,i);
	return *this;
}

Msg& Msg::reply(Msg& req) {
//	INFO("%s",this->toString().c_str());
	std::string s="request";
	uint32_t rq;
	rq=req.cls();
	const char* sRq = Uid::label(rq);
	if ( sRq !=0 ) s=sRq;
	else WARN(" no label %u ",rq);
	s += "Reply";
	dst(req.src());
	src(req.dst());
	cls(Uid::add(s.c_str()));
	id(req.id());
	return *this;
}

Msg& Msg::src(uid_type uid) {poke(OFF_SRC,uid);	return* this;}
Msg& Msg::dst(uid_type uid) {poke(OFF_DST,uid);	return *this;}
Msg& Msg::cls(uid_type uid) {poke(OFF_CLS,uid);	return *this;}

uid_type Msg::dst() {	return peek(OFF_DST);}
uid_type Msg::src() {	return peek(OFF_SRC);}
uid_type Msg::cls() {	return peek(OFF_CLS);}

Msg::~Msg() {
//	INFO("dtor %X : [%d]",this,capacity());
}

Msg& Msg::operator=(const Msg& src) {
	(Xdr&)*this = (const Xdr& )src;
	return *this;
}



//_________________________________________________ Actor


MsgClass Actor::ReceiveTimeout() {
	static MsgClass m("ReceiveTimeout");
	return m;
}
MsgClass Actor::PoisonPill() {
	static MsgClass m("PoisonPill");
	return m;
}

MsgClass Actor::AnyClass() {
	static MsgClass m("AnyClass");
	return m;
}

MsgClass Actor::Properties() {
	static MsgClass m("properties");
	return m;
}

MsgClass Actor::PropertiesReply() {
	static MsgClass m("propertiesReply");
	return m;
}
list<Actor*> Actor::_actors;

Actor::Actor() : _context() {}

Actor::~Actor() {}

ActorRef& Actor::self() { return _context->self(); }

Receive& Actor::receiveBuilder() { return *(new Receive()); }

Msg& Actor::msgBuilder(MsgClass cls) { return context().dispatcher().txd().clear().cls(cls.id());}

Msg& Actor::replyBuilder(Msg& msg) { return msgBuilder("0").reply(msg); }

ActorContext& Actor::context() { return *_context; }

TimerScheduler& Actor::timers() { return _context->timers(); }

void Actor::unhandled(Msg& msg) {
	WARN(" unhandled : '%s'=>'%s'=>'%s'",Uid::label(msg.src()),Uid::label(msg.cls()),Uid::label(msg.dst()));
}

ActorRef& Actor::sender() { return _context->sender(); }

//Msg& Actor::txdMsg() { return context().dispatcher().txdMsg(); }

//_______________________________________________ ActorRef

ActorRef::ActorRef() : Uid(ActorRef::NoSender()) {}

ActorRef& ActorRef::NoSender() {
	static ActorRef ref("NoSender");
	return ref;
}

unordered_map<uid_type, ActorRef*>* ActorRef::_actorRefs;

ActorRef* ActorRef::lookup(Uid uid) {
	std::unordered_map<uid_type, ActorRef*>::const_iterator got =
	    actorRefs()->find(uid.id());
	if (got != actorRefs()->end())
		return got->second;
	return &NoSender();
}

ActorRef::ActorRef(Uid uid, Mailbox* mailbox) : Uid(uid) {
//	printf(" created ActorRef '%s' = %d , mailbox : %X ", uid.label(),uid.id(),mailbox);
	if (actorRefs()->find(uid.id()) == actorRefs()->end())
		actorRefs()->emplace(uid.id(), this);
	_cell = 0;
	_mailbox = mailbox;
}

ActorRef::ActorRef(Uid uid) : Uid(uid) {
	INFO(" created ActorRef '%s' = %d", uid.label(),uid.id());
	if (actorRefs()->find(uid.id()) == actorRefs()->end())
		actorRefs()->emplace(uid.id(), this);
	_cell = 0;
	_mailbox = 0;
}

bool ActorRef::isLocal() { return _cell == 0; }

const char* ActorRef::path() { return label(); }

Mailbox& ActorRef::mailbox() { return *_mailbox; }
void ActorRef::mailbox(Mailbox& mb) { _mailbox = &mb; }

bool ActorRef::operator==(ActorRef v) { return id() == v.id(); }
//______________________________________________ LocalActorRef

void ActorRef::tell(  Msg& msg,ActorRef src) {

	msg.src(src.id());
	msg.dst(this->id());
//	msg.add(UID_ID,Msg::newId());

	if (_mailbox != 0)
		_mailbox->enqueue(msg);
	else {
		WARN(" no mailbox attached to ActorRef %s ", label());
	}
}

void ActorRef::tell(Msg& msg ) {
	if (_mailbox != 0)
		_mailbox->enqueue(msg);
	else {
		WARN(" no mailbox attached to ActorRef %s ", label());
	}
}

void ActorRef::forward(Msg& msg) {
	msg.dst(this->id());
	if (_mailbox != 0)
		_mailbox->enqueue(msg);
	else {
		WARN(" no mailbox attached to ActorRef %s ", label());
	}
}
void ActorRef::cell(ActorCell* cell) { _cell = cell; }

ActorCell* ActorRef::cell() { return _cell; }

//____________________________________________________________ ActorSelection

ActorSelection::ActorSelection(Uid id) : ActorRef(id, 0) {}


typedef std::function<void(Msg&)> MessageHandler;

//________________________________________________________ Mailbox
//

Mailbox::Mailbox(const char* name, uint32_t queueSize) : _name(name) {
	_queue = xQueueCreate(100,sizeof(Msg*));
}

int Mailbox::enqueue(Msg& msg) {
//	INFO(" enqueue : %s ",msg.toString().c_str());
	Msg* px = new Msg(msg.size());
	*px = msg;
	configASSERT(msg.src()!=0);
	configASSERT(msg.dst()!=0);
	BaseType_t rc =  xQueueSend(_queue,&px,10);
	if ( rc != pdTRUE ) {
		WARN("enqueue failed %d for %s ",rc,msg.toString().c_str());
		delete px;
		return ENOENT;
	}
	return 0;
}


int Mailbox::dequeue(Msg& msg,uint32_t time) {
	Msg* px;
	if ( xQueueReceive(_queue,&px,pdMS_TO_TICKS(time))!=pdTRUE) {
		return ENOENT;
	}
	(Msg&)msg = *px;
	delete px;
	return 0;
}

//_______________________________________________________ ActorSystem
//
ActorSystem::ActorSystem(const char* name, MessageDispatcher& defaultDispatcher,
                         Mailbox& defaultMailbox)
	: Uid(name), _name(name),_defaultProps(defaultDispatcher, defaultMailbox) {
	Uid::add(AKKA_DST);
	Uid::add(AKKA_SRC);
	Uid::add(AKKA_CLS);
	Uid::add(AKKA_ID);
	Uid::add(AKKA_TIMER);
}


ActorRef& ActorSystem::actorFor(const char* address) {
	// TODO check local or remote
	ActorRef* ref = ActorRef::lookup(Uid::add(address));
	if (ref == 0)
		ref = new ActorRef(address, &_defaultProps.mailbox());
	return *ref;
}

ActorRef* ActorSystem::create(Actor* actor, const char* name, Props& props) {
	std::string path=label();
	path+="/";
	path += name;
	Uid uid = Uid::add(path.c_str());
	ActorRef* actorRef;
	if (!(*ActorRef::lookup(uid.id())== ActorRef::NoSender() )) {
		WARN(" actor with '%s'this'' name already exist , not created",name);
		return 0;
	}
	actorRef = new ActorRef(uid, &props.mailbox());
	ActorCell* actorCell = new ActorCell(*this, *actorRef, props.mailbox(),
	                                     props.dispatcher());
	actorRef->cell(actorCell);
	actorCell->actor(actor);
	actor->context(actorCell);
	actor->preStart();
	actorCell->become(actor->createReceive(), true);
	INFO(" new actor '%s'[%u] created", actorRef->path(),actorRef->id());
	props.dispatcher().attach(*actorCell);
	_actorRefs.push_back(actorRef);
	return actorRef;
}


//___________________________________________________________ Receive

Receive Receive::NullReceive;
//___________________________________________________________ Receiver
//
Receiver::Receiver(MsgClass msgClass, MessageMatcher matcher,
                   MessageHandler handler)
	: _msgClass(msgClass), _matcher(matcher), _handler(handler) {}

Receiver::Receiver(MsgClass msgClass, MessageHandler handler)
	: _msgClass(msgClass), _matcher(alwaysTrue), _handler(handler) {}

inline void Receiver::onMessage(Msg& msg) { _handler(msg); }
bool Receiver::match(Msg& msg) {
	if (_msgClass.id() == msg.cls() || _msgClass == Actor::AnyClass() )
		return (_matcher(msg));
	return false;
}

string& Receiver::toString(string& s) {
	string_format(s, " class : %s  ", _msgClass.label());
	return s;
}
//_____________________________________________ Timer

void Timer::callBack(TimerHandle_t handle) {
	Timer* timer=(Timer*) pvTimerGetTimerID(handle);
//	INFO(" timer call back %s ",timer->label());
	timer->_timerScheduler.timerCallback(*timer);
}

Timer::Timer(Uid key,  bool autoReload, uint32_t interval,const Msg& m,TimerScheduler& scheduler)
	: Uid(key),  _timerScheduler(scheduler) {
	INFO("[%X] timer created %s : %u ",this,label(),interval);
	_msg = new Msg();
	*_msg = m;
	_msg->dst(scheduler.ref().id());
	_msg->src(scheduler.ref().id());
	_autoReload=autoReload;
	configASSERT((_timer = xTimerCreate(label(),pdMS_TO_TICKS(interval),autoReload,this,callBack))!=NULL);
	start();
}


void Timer::start() {
//	INFO("[%X] timer start",this);
	configASSERT( xTimerStart(_timer,0) == pdPASS ) ;
}

void Timer::stop() {
//	INFO("[%X] timer stop",this);
	configASSERT(xTimerStop(_timer,0)==pdPASS);
}

Timer::~Timer() {
	INFO("[%X] timer dtor",this);
	xTimerDelete(_timer,0);
	delete _msg;
}

Msg& Timer::msg() {
	return *_msg;
}

void Timer::msg(const Msg& msg) {
//	if ( _msg != 0 ) delete _msg;
	*_msg = msg;
	_msg->dst(_timerScheduler.ref().id());
	_msg->src(_timerScheduler.ref().id());
}

void Timer::interval(uint32_t interv) {
	INFO("[%X] timer interval(%u)",this,interv);
	configASSERT(xTimerDelete(_timer,0)==pdPASS);
	configASSERT((_timer = xTimerCreate(label(),pdMS_TO_TICKS(interv),_autoReload,this,callBack))!=NULL);
	start();
}


Uid Timer::key() { return id(); }

//__________________________________________________________ TimerScheduler

void TimerScheduler::timerCallback(Timer& timer) {
//	INFO(" timer call back : %s %s ",timer.label(),_ref.label());
	_ref.tell(timer.msg());
}

TimerScheduler::TimerScheduler(ActorRef ref) :_ref(ref) {}

Timer* TimerScheduler::find(Uid key) {
	for (Timer* timer : _timers) {
		if (timer->key() == key)
			return timer;
	}
	return 0;
}



Uid TimerScheduler::startPeriodicTimer(Uid key,const Msg& msg, uint32_t msec) {
	Timer* timer = find(key.id());

	if (timer == 0) {
		_timers.push_back(new Timer(key,  true, msec, msg,*this));
	} else {
		timer->msg(msg);
		timer->start();
	}
	return key.id();
}

Uid TimerScheduler::startSingleTimer(Uid key, const Msg& msg, uint32_t msec) {
	Timer* timer = find(key.id());

	if (timer == 0) {
		_timers.push_back(new Timer(key,  false, msec,msg,*this));
	} else {
		timer->msg(msg);
		timer->start();
	}
	return key.id();
}

void TimerScheduler::cancel(Uid key) {
	Timer* timer = find(key);
	if (timer)
		timer->stop();
	else
		WARN(" timer.cancel()  key not found : %s ", key.label());
}

void TimerScheduler::cancelAll() {
	for (Timer* t : _timers) {
		t->stop();
	}
}

ActorRef& TimerScheduler::ref() {
	return _ref;
}


//___________________________________________________________________ Receive
//

Receive::Receive() {}

Receive& Receive::match(MsgClass msgClass, MessageHandler doSome) {
	_receivers.push_back(new Receiver(msgClass, Receiver::alwaysTrue, doSome));
	return *this;
}

Receive& Receive::build() { return *this; }

Receive& Receive::receive(ActorCell& cell,Msg& msg) {
	bool found=false;

	for (auto receiver : _receivers) {
		if (receiver->match(msg)) {
			receiver->onMessage(msg);
			found=true;
		}
	}
	if ( !found ) {
		cell.unhandled(msg);
	}
	return *this;
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
//	INFO(" ActorCell created %s [%d]",_self.path(),sizeof(ActorCell));
}

const char* ActorCell::path() { return _self.path(); }

Mailbox& ActorCell::mailbox() { return _mailbox; }

ActorRef& ActorCell::self() { return _self; }

uint64_t ActorCell::expiresAt() { return _lastReceive + _inactivityPeriod; }

MessageDispatcher& ActorCell::dispatcher() { return _dispatcher; }

ActorCell* ActorCell::lookup(ActorRef* ref) { return ref->cell(); }

void ActorCell::actor(Actor* actor) { _actor=actor;};

void ActorCell::invoke(Msg& msg) {
	_currentMessage = &msg;
	_receive->receive(*this,msg);
}

void ActorCell::unhandled(Msg& envelope) {
	_actor->unhandled(envelope);
}

ActorRef& ActorCell::sender() { return *ActorRef::lookup(_currentMessage->src()); }

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
		_receive = &Receive::NullReceive;
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
		_timers = new TimerScheduler(self());
	return *_timers;
}

bool ActorCell::hasTimers() { return _timers != 0; }

list<ActorCell*>& ActorCell::actorCells() { return _actorCells; }

//________________________________________________ MessageDispatcher
//

MessageDispatcher::MessageDispatcher() {
	_unhandledCell = 0;
}

void MessageDispatcher::attach(Mailbox& mailbox) {
//	_mailboxes.push_back(&mailbox);
	_mailbox = &mailbox;
}

void MessageDispatcher::attach(ActorCell& cell) {
	_actorCells.push_back(&cell);
}

void MessageDispatcher::unhandled(ActorCell* cell) { _unhandledCell = cell; }

//Msg& MessageDispatcher::txdMsg() { return _txdMsg; }

void MessageDispatcher::nextWakeup(uint64_t t) {
	//   INFO(" %lld ", t);
	if (_nextWakeup > t)
		_nextWakeup = t;
}

uint64_t MessageDispatcher::nextWakeup() { return _nextWakeup; }

extern bool isTask();

void MessageDispatcher::execute(bool loop) {
	ActorRef* sender;
	ActorRef* receiver;
	MsgClass msgClass;
	uid_type uid;
	uint32_t id;
	while(loop) {
		while( _mailbox->dequeue(_rxd,100)==0) { // no message*/
			uid=_rxd.dst();
			receiver = ActorRef::lookup(uid);
			if (*receiver==ActorRef::NoSender()) WARN("src ? %s",_rxd.toString().c_str());

			uid = _rxd.src();
			sender = ActorRef::lookup(uid);
			if (*sender==ActorRef::NoSender()) WARN("dst ? %s",_rxd.toString().c_str());

			uid = _rxd.cls();
			msgClass = MsgClass(uid);
			if (uid==0) WARN("cls ? %s",_rxd.toString().c_str());

			id = _rxd.id();


//			INFO("%s",_rxdMsg.toString().c_str());

			ActorCell* cell = receiver->cell();
			if (cell) {
				uint64_t startTime = Sys::millis();
				cell->invoke(_rxd);
				uint32_t delta = Sys::millis() - startTime ;
				if ( delta > 10 ) {
					WARN(" slow actor %d msec : '%s' on '%s' ",delta,cell->self().label(),Uid::label(_rxd.cls()));
				}
				cell->resetReceiveTimeout();
			} else {
				if (_unhandledCell) {
					_unhandledCell->invoke(_rxd);
				} else {
					WARN(" no Receive found  %s->%s:%s ",
					     sender->path(),
					     receiver->path(),
					     msgClass.label());
				}
			}
		}
//		INFO("");
	}
}

Msg& MessageDispatcher::txd() { return _txd; };
// void arduinoLoop() { defaultDispatcher.execute(); }
