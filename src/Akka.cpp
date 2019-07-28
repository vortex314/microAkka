//============================================================================
// Name        : akkaMicro.cpp
// Author      : Lieven
// Version     :
// Copyright   : Enjoy the source
// Description : Akka alike framework in C++ for embedded systems : low RAM
//============================================================================
#include "Akka.h"

MsgClass SignalFromIsr(UID("SignalFromIsr"));
//_____________________________________________ STATIC

typedef void (*MsgHandler)(void);
const char* cloneString(const char* s) {
	char* ps = new char[strlen(s) + 1];
	strcpy(ps, s);
	return (const char*) ps;
}

//_______________________________________________ Uid
//
/*
std::unordered_map<uid_type, Uid::UidStruct*>* Uid::_labels;

std::unordered_map<uid_type, Uid::UidStruct*>* Uid::labels()
{
    if (_labels == 0) {
        _labels = new std::unordered_map<uid_type, UidStruct*>();
    }
    return _labels;
}

Uid::Uid(const char* label)
    : Uid(H(label), label)
{
}

Uid::Uid(uid_type id)
    : Uid(id, "unknown")
{
}

Uid::Uid()
    : Uid(H("unknown"))
{

}

bool Uid::operator==(Uid& other)
{
    return other._pl->_uid == _pl->_uid;
}

uid_type Uid::id()
{
    return _pl->_uid;
}
const char* Uid::label()
{
    if (_pl->_label) return _pl->_label;
    return "unknown";
}

const char* Uid::label(uid_type uid)
{
    std::unordered_map<uid_type, UidStruct*>::const_iterator p = labels()
            ->find(uid);
    if (p == labels()->end()) return "NONE";
    return (p->second->_label);
}

const char* uidToString(uid_type uid)
{
    return Uid::label(uid);
}

//_______________________________________________ Ref
//
std::unordered_map<uid_type, Ref::RefStruct*> Ref::_refs;
Ref Ref::NotFound(0);

Ref::~Ref()
{

}

const char* Ref::label()
{
    myASSERT(_pr);
    return _pr->_label.label();
}

uid_type Ref::id()
{
    return _pr->_label.id();
}

Ref Ref::findRef(uid_type id,uid_type cls)
{
    auto r = _refs.find(id);
    if (r == _refs.end()) return 0;
    if ( r->second->_cls.id() != cls) return 0;
    return r->second;
}

Uid Ref::cls()
{
    return _pr->_cls;
}

void* Ref::object()
{
    return _pr->_object;
}

bool Ref::operator==(Ref& that)
{
    return this->_pr == that._pr;
}
*/
//_________________________________________________ MsgClass

//_________________________________________________ Msg

Msg::Msg()
	: Xdr(12) {
//	INFO("ctor %X : [%d]",this,capacity());
	add(UD_CLS, (uid_type) 0);
	add(UD_SRC, (uid_type) 0);
	add(UD_DST, (uid_type) 0);
}

Msg::Msg(const Msg& src)
	: Xdr((Xdr&) src) {
}

Msg::Msg(uint32_t size)
	: Xdr(size) {
//	INFO("ctor %X : [%d]",this,capacity());
	add(UD_CLS, (uid_type) 0);
	add(UD_SRC, (uid_type) 0);
	add(UD_DST, (uid_type) 0);
}

Msg::Msg(MsgClass cls)
	: Xdr(12) {
//	INFO("ctor %X : [%d]",this,capacity());

	add(UD_CLS, cls.id());
	add(UD_SRC, (uid_type) 0);
	add(UD_DST, (uid_type) 0);
}
Msg::Msg(Uid cls, Uid src)
	: Xdr(12) {
//	INFO("ctor %X : [%d]",this,capacity());
	add(UD_CLS, cls.id());
	add(UD_SRC, src.id());
	add(UD_DST, (uid_type) 0);
}

Msg& Msg::clear() {
	Xdr::clear();
	add(UD_CLS, (uid_type) 0);
	add(UD_SRC, (uid_type) 0);
	add(UD_DST, (uid_type) 0);
	return *this;
}

uint32_t Msg::id() {
	uint32_t id = 0;
	get(UD_ID, id);
	return id;
}

Msg& Msg::id(uint32_t i) {
	add(UD_ID, i);
	return *this;
}

Msg& Msg::reply(Msg& req) {
//	INFO("%s",this->toString().c_str());
	std::string s = "request";
	uint32_t rq;
	rq = req.cls();
	const char* sRq = Uid::label(rq);
	if (sRq != 0)
		s = sRq;
	else WARN(" no label %u ", rq);
	s += "Reply";
	dst(req.src());
	src(req.dst());
	cls(Uid(s.c_str()).id());
	id(req.id());
	return *this;
}

Msg& Msg::src(uid_type uid) {
	poke(OFF_SRC, uid);
	return *this;
}
Msg& Msg::dst(uid_type uid) {
	poke(OFF_DST, uid);
	return *this;
}
Msg& Msg::cls(uid_type uid) {
	poke(OFF_CLS, uid);
	return *this;
}

uid_type Msg::dst() {
	return peek(OFF_DST);
}
uid_type Msg::src() {
	return peek(OFF_SRC);
}
uid_type Msg::cls() {
	return peek(OFF_CLS);
}

Msg::~Msg() {
//	INFO("dtor %X : [%d]",this,capacity());
}

Msg& Msg::operator=(const Msg& src) {
	(Xdr&) *this = (const Xdr&) src;
	return *this;
}

std::string Msg::toString() {
	std::string result;
	result += Uid::label(src());
	result += " = ";
	result += Uid::label(cls());
	result += "> ";
	result += Uid::label(dst());
//	result+= Xdr::toString();
	return result;
}

//_________________________________________________ MsgClass

MsgClass MsgClass::ReceiveTimeout() {
	static MsgClass m("ReceiveTimeout");
	return m;
}
MsgClass MsgClass::PoisonPill() {
	static MsgClass m("PoisonPill");
	return m;
}

MsgClass MsgClass::AnyClass("AnyClass");

MsgClass MsgClass::Properties() {
	static MsgClass m("Properties");
	return m;
}

MsgClass MsgClass::PropertiesReply() {
	static MsgClass m("PropertiesReply");
	return m;
}

//_________________________________________________ Actor

std::list<Actor*> Actor::_actors;

Actor::Actor()
	: _context() {
}

void Actor::context(ActorCell* context) {
	_context = context;
}

Actor::~Actor() {
	WARN(" actor '%s' delete", self().path());
}

ActorRef& Actor::self() {
	return _context->self();
}

Receive& Actor::receiveBuilder() {
	return *(new Receive());
}

Msg& Actor::msgBuilder(MsgClass cls) {
	return context().currentThread().txd().clear().cls(cls.id());
}

Msg& Actor::replyBuilder(Msg& msg) {
	return msgBuilder("0").reply(msg);
}

ActorContext& Actor::context() {
	return *_context;
}

TimerScheduler& Actor::timers() {
	return _context->timers();
}

void Actor::unhandled(Msg& msg) {
	WARN(" unhandled : '%s'=>'%s'=>'%s'", Uid::label(msg.src()), Uid::label(msg
	        .cls()), Uid::label(msg.dst()));
}

ActorRef& Actor::sender() {
	return _context->sender();
}

//Msg& Actor::txdMsg() { return context().dispatcher().txdMsg(); }

//_______________________________________________ ActorRef
//

std::unordered_map<uid_type,ActorRef*> ActorRef::_actorRefs;

ActorRef::ActorRef(Uid label)
	: Uid(label) {
	DEBUG(" created ActorRef '%s' = %d", label.label(), label.id());
	if ( _actorRefs.find(label.id())==_actorRefs.end())
		_actorRefs.emplace(label.id(),this);
}

ActorRef::~ActorRef() {
	WARN(" ActorRef destroyed : %s ",path());
	_actorRefs.erase(id());
}

bool ActorRef::operator==(ActorRef& v) {
	return id() == v.id();
}

const char* ActorRef::path() {
	return label();
}

//_______________________________________________ LocalActorRef
LocalActorRef::LocalActorRef(Uid label, ActorSystem& system, Props& props,
                             MessageDispatcher& dispatcher)
	: ActorRef(label), _cell(*new ActorCell(system, *this, dispatcher, props)) {
}

LocalActorRef::~LocalActorRef() {
	delete &_cell;
}

ActorRef* ActorRef::lookup(uid_type uid) {
	auto search = _actorRefs.find(uid);
	if (search == _actorRefs.end()) return 0;
	return search->second;
}

//______________________________________________ LocalActorRef

void LocalActorRef::tell(Msg& msg, ActorRef& src) {
	msg.src(src.id());
	msg.dst(this->id());
	_cell.sendMessage(msg);
}

void LocalActorRef::tell(Msg& msg) {
	msg.dst(this->id());
	_cell.sendMessage(msg);
}

Msg isrMsg(SignalFromIsr);
void LocalActorRef::signalFromIsr(uint32_t signal) {
//	static Msg msg(SignalFromIsr);
	isrMsg.id(signal);
	isrMsg.dst(this->id());
	_cell.sendMessageFromIsr(isrMsg);
}

void LocalActorRef::forward(Msg& msg, ActorContext& context) {
	//TODO define
}

bool LocalActorRef::isLocal() {
	return true;
}

Mailbox& LocalActorRef::mailbox() {
	return _cell.mailbox();
}

ActorCell& LocalActorRef::cell() {
	return _cell;
}

//____________________________________________________________ EmptyLocalActorRef
//
ActorRef& ActorRef::NoSender() {
	static EmptyLocalActorRef noSender;
	return noSender;
}
//____________________________________________________________ RemoteActorRef
//

RemoteActorRef::RemoteActorRef(Uid label, ActorRef& bridge)
	: ActorRef(label), _bridge(bridge) {
}

void RemoteActorRef::tell(Msg& msg) {
	msg.dst(this->id());
//			INFO(" remoting %s ",msg.toString().c_str());
	((LocalActorRef&) _bridge).cell().sendMessage(msg);
//			_bridge.tell(msg);
}
void RemoteActorRef::tell(Msg& msg, ActorRef& src) {
	msg.src(src.id());
	msg.dst(this->id());
//			INFO(" remoting %s ",msg.toString().c_str());
	((LocalActorRef&) _bridge).cell().sendMessage(msg);
}
void RemoteActorRef::forward(Msg& message, ActorContext& context) {
}

bool RemoteActorRef::operator==(ActorRef& src) {
	return id() == src.id();
}

bool RemoteActorRef::isLocal() {
	return false;
}

void RemoteActorRef::signalFromIsr(uint32_t signal) {
}


//____________________________________________________________ ActorSelection

typedef std::function<void(Msg&)> MessageHandler;

//________________________________________________________ Mailbox
//

Mailbox::Mailbox(ActorCell& cell, uint32_t queueSize)
	: NativeQueue(queueSize), _cell(cell) {
	_currentStatus = Open;
}

int Mailbox::enqueue(Msg& msg) {
	Msg* px = new Msg(msg.size());
	*px = msg;
	if (msg.cls() == 0)
		WARN(" msg.cls==0 to %s from %s ", Uid::label(msg.dst()), Uid::label(msg
		        .src()));
	int rc = send(px, 10);
	if (rc != 0) {
		WARN("'%s' enqueue failed to '%s'", Uid::label(msg.cls()), name());
		delete px;
		return rc;
	}
	return 0;
}

int Mailbox::enqueueFromIsr(Msg& msg) {
	int rc = sendFromIsr(&msg);
	if (rc != 0) {
		WARN("[%X] enqueue failed %s", this, Uid::label(_cell.self().id()));
		return ENOENT;
	}
	return 0;
}

int Mailbox::dequeue(Msg& msg, uint32_t time) {
	Msg* px;
	int rce = recv(&px, time);
	if (rce) {
		DEBUG("'%s' dequeue failed : %d", name(), rce);
		return ENOENT;
	};
	(Msg&) msg = *px;
	if (msg.cls() != SignalFromIsr.id()) delete px;
	return 0;
}

// for now supposing mailbox always open
bool Mailbox::canBeScheduledForExecution(bool hasMessagesHint) {
	if ((_currentStatus == Scheduled) || (_currentStatus == Open))
		return hasMessagesHint || messageCount() > 0;
	if (_currentStatus == Closed) return false;
	return false;
}
// check suspend counter, not used here in fact
bool Mailbox::shouldProcessMessage() {
	return (_currentStatus & shouldNotProcessMask) == 0;
}
//if already scheduled==true return false, else set scheduled, indicates winning thread
bool Mailbox::setAsScheduled() {
	DEBUG("'%s' setAsScheduled", name());
	while (true) {
		uint32_t s = _currentStatus;
		if ((s & shouldScheduleMask) != Open) return false;
		if (updateStatus(s, s | Scheduled)) return true;
	}
}
// set scheduled false
bool Mailbox::setAsIdle() {
	DEBUG("'%s' setAsIdle", name());
	while (true) {
		uint32_t s = _currentStatus;
		if (updateStatus(s, s & ~Scheduled)) return true;
	}
}

bool Mailbox::updateStatus(uint32_t oldStatus, uint32_t newStatus) {
	// no std::atomic in ESP8266 libstdc++
	// let's do it the hard way
#ifdef ESP_OPEN_RTOS
	bool b=false;
	taskENTER_CRITICAL();
	if ( _currentStatus == oldStatus ) {
		_currentStatus=newStatus;
		b=true;
	} else {
		b=false;
	}
	taskEXIT_CRITICAL();
	return b;
#else
	uint32_t expected = oldStatus;
	bool b =
	    std::atomic_compare_exchange_weak<uint32_t>(&_currentStatus, &oldStatus, newStatus);
	if (!b) {
		DEBUG(" value different %u <> %u", expected, oldStatus);
	}
	return b;
#endif
}

void Mailbox::processMailbox(Thread* thread) {
	if (shouldProcessMessage()) {
		uint32_t counter = 0;
		while (dequeue(thread->rxd(), 0) == 0) {
			_cell.currentThread(thread);
			_cell.invoke(thread->rxd());
			counter++;
			if (counter == 10) break;
		}
		if (counter > 3)
			DEBUG("'%s' processed %d messages ", _cell.path(), counter);
		_cell.resetReceiveTimeout();
		setAsIdle();
		thread->dispatcher().registerForExecution(this, false);
	}
}

const char* Mailbox::name() {
	return _cell.path();
}

//_______________________________________________________ ActorSystem
//
ActorSystem::ActorSystem(Uid label, MessageDispatcher& defaultDispatcher)
	: Uid(label), _defaultDispatcher(defaultDispatcher) {
	Uid(AKKA_DST);
	Uid(AKKA_SRC);
	Uid(AKKA_CLS);
	Uid(AKKA_ID);
	Uid(AKKA_TIMER);
	_defaultDispatcher.start();
}

ActorRef* ActorSystem::create(Actor* actor, const char* name, Props& props) {
	std::string path = label();
	path += "/";
	path += name;
	Uid label(path.c_str());
	LocalActorRef* localActorRef;
	if (!(ActorRef::lookup(label.id()) == 0)) {
		WARN(" actor with '%s'this'' name already exist , not created", name);
		return 0;
	}

	localActorRef = new LocalActorRef(label, *this, props, _defaultDispatcher);
	ActorCell& actorCell = localActorRef->cell();
	actorCell.actor(actor);
	actor->context(&actorCell);
	actor->preStart();
	actorCell.become(actor->createReceive(), true);
	DEBUG("[%u] actor '%s'created", localActorRef->id(), localActorRef->path());
	_actorRefs.emplace(localActorRef->id(),localActorRef);
//	props.dispatcher().attach(actorCell);
	return localActorRef;
}

MessageDispatcher& ActorSystem::defaultDispatcher() {
	return _defaultDispatcher;
}

std::unordered_map<uid_type,ActorRef*>& ActorSystem::actorRefs() {
	return _actorRefs;
}

//___________________________________________________________ Receive

Receive Receive::NullReceive;
//___________________________________________________________ Receiver
//
Receiver::Receiver(MsgClass msgClass, MessageMatcher matcher,
                   MessageHandler handler)
	: _msgClass(msgClass), _matcher(matcher), _handler(handler) {
}

Receiver::Receiver(MsgClass msgClass, MessageHandler handler)
	: _msgClass(msgClass), _matcher(alwaysTrue), _handler(handler) {
}

inline void Receiver::onMessage(Msg& msg) {
	_handler(msg);
}
bool Receiver::match(Msg& msg) {
	if (_msgClass.id() == msg.cls() || _msgClass == MsgClass::AnyClass)
		return (_matcher(msg));
	return false;
}

std::string& Receiver::toString(std::string& s) {
	string_format(s, " class : %s  ", _msgClass.label());
	return s;
}
//_____________________________________________ Timer

void Timer::callBack(void* arg) {
	Timer* me = (Timer*) arg;
//	INFO(" timer call back %s ",timer->label());
	me->_timerScheduler.timerCallback(*me);
}

Timer::Timer(Uid key, bool autoReload, uint32_t interval, const Msg& m,
             TimerScheduler& scheduler)
	: Uid(key), NativeTimer(key.label(), autoReload, interval, this, callBack), _timerScheduler(scheduler) {
	DEBUG("[%X] timer created %s : %u ", this, label(), interval);
	_msg = new Msg();
	*_msg = m;
	_msg->dst(scheduler.ref().id());
	_msg->src(scheduler.ref().id());
	_autoReload = autoReload;
	start();
}

Timer::~Timer() {
	DEBUG("[%X] timer dtor", this);
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

Uid Timer::key() {
	return id();
}

//__________________________________________________________ TimerScheduler

void TimerScheduler::timerCallback(Timer& timer) {
//	INFO(" timer call back : %s %s ",timer.label(),_ref.label());
	/*	if ( timer.msg().cls() == MsgClass::ReceiveTimeout()) {
	 ActorCell cell;
	 if ( Sys::millis() > (cell.lastReceive()+ cell.receiveTimeout()) ) {
	 timer.
	 } else {

	 }
	 }*/
	_ref.tell(timer.msg());
}

TimerScheduler::TimerScheduler(ActorRef& ref)
	: _ref(ref) {
}

Timer* TimerScheduler::find(Uid key) {
	for (Timer* timer : _timers) {
		if (timer->key() == key) return timer;
	}
	return 0;
}

Uid TimerScheduler::startPeriodicTimer(Uid key, const Msg& msg,
                                       uint32_t msec) {
	Timer* timer = find(key.id());

	if (timer == 0) {
		_timers.push_back(new Timer(key, true, msec, msg, *this));
	} else {
		timer->msg(msg);
		timer->start();
	}
	return key.id();
}

Uid TimerScheduler::startSingleTimer(Uid key, const Msg& msg,
                                     uint32_t msec) {
	Timer* timer = find(key.id());

	if (timer == 0) {
		_timers.push_back(new Timer(key, false, msec, msg, *this));
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
	for (Timer* timer : _timers) {
		timer->stop();
	}
}

ActorRef& TimerScheduler::ref() {
	return _ref;
}

//___________________________________________________________________ Receive
//

Receive::Receive() {
}

Receive& Receive::match(MsgClass msgClass, MessageHandler doSome) {
	_receivers.push_back(new Receiver(msgClass, Receiver::alwaysTrue, doSome));
	return *this;
}

Receive& Receive::build() {
	return *this;
}

Receive& Receive::receive(ActorCell& cell, Msg& msg) {
	bool found = false;

	for (auto receiver : _receivers) {
		if (receiver->match(msg)) {
			receiver->onMessage(msg);
			found = true;
		}
	}
	if (!found) {
		cell.unhandled(msg);
	}
	return *this;
}

//______________________________________________________________ ActorCell
//

std::list<ActorCell*> ActorCell::_actorCells;

ActorCell::ActorCell(ActorSystem& system, ActorRef& ref,
                     MessageDispatcher& dispatcher, Props& props)
	: _mailbox(*new Mailbox(*this, 15)), _dispatcher(dispatcher), _system(system), _self(ref)
//, _receiveTimeoutTimer(ref, true, UINT32_MAX- 1, Msg(MsgClass::ReceiveTimeout()), timers())
{
	_enable = true;
	_currentThread = 0;
	_timers = 0;
	_lastReceive = UINT64_MAX - 2 * UINT32_MAX;
	_inactivityPeriod = UINT32_MAX;
	_receive = 0;
	_prevReceive = 0;
	_actorCells.push_back(this);
	/*	_semaphore = xSemaphoreCreateBinary();
	 xSemaphoreGive(_semaphore);*/
	_actor = 0;
	INFO(" ActorCell created %s mailbox [%X]", _self.path(), &_mailbox);
}

ActorCell::~ActorCell() {
	//_dispatcher->detach(*this);
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

uint64_t ActorCell::expiresAt() {
	return _lastReceive + _inactivityPeriod;
}

void ActorCell::actor(Actor* actor) {
	_actor = actor;
}

void ActorCell::invoke(Msg& msg) {
	/*	while (xSemaphoreTake(_semaphore, (TickType_t)1) != pdTRUE) {
	 printf(" xSemaphoreTake()  timed out ");
	 }*/
	if (_receive)
		_receive->receive(*this, msg);
	else WARN(" no receive defined for %s for message %s ", self().path(), msg
		          .toString().c_str());
	/*	if (xSemaphoreGive(_semaphore) != pdTRUE) {
	 printf("xSemaphoreGive() failed");
	 }*/
}

void ActorCell::sendMessage(Msg& msg) {
	_dispatcher.dispatch(*this, msg);
}

void ActorCell::sendMessageFromIsr(Msg& msg) {
	_dispatcher.dispatchFromIsr(*this, msg);
}

void ActorCell::unhandled(Msg& envelope) {
	_actor->unhandled(envelope);
}

ActorRef& ActorCell::sender() {
	return *ActorRef::lookup(_currentThread->currentMessage().src());
}

ActorSystem& ActorCell::system() {
	return _system;
}
Thread& ActorCell::currentThread() {
	return *_currentThread;
}

void ActorCell::currentThread(Thread* t) {
	_currentThread = t;
}

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
//	_receiveTimeoutTimer.interval(msec);
}

uint32_t ActorCell::receiveTimeout() {
	return _inactivityPeriod;
}

void ActorCell::resetReceiveTimeout() {
	_lastReceive = Sys::millis();
//	_receiveTimeoutTimer.reset();
}

bool ActorCell::hasReceiveTimedOut() {
	return Sys::millis() > (_lastReceive + _inactivityPeriod);
}

TimerScheduler& ActorCell::timers() {
	if (_timers == 0) _timers = new TimerScheduler(self());
	return *_timers;
}

bool ActorCell::hasTimers() {
	return _timers != 0;
}

std::list<ActorCell*>& ActorCell::actorCells() {
	return _actorCells;
}

//________________________________________________ Thread

Thread::Thread(MessageDispatcher* dispatcher, const char* name,
               uint32_t stackSize, uint32_t priority, void* threadArg, TaskFunction f)
	:Uid(name), NativeThread(name, stackSize, priority, this, f), _dispatcher(dispatcher) {

}

Msg& Thread::txd() {
	return _txd;
}
Msg& Thread::rxd() {
	return _rxd;
}

MessageDispatcher& Thread::dispatcher() {
	return *_dispatcher;
}
Msg& Thread::currentMessage() {
	return _rxd;
}

//________________________________________________ MessageDispatcher
//

MessageDispatcher::MessageDispatcher(uint32_t threadCount, uint32_t stackSize,
                                     uint32_t priority)
	: _workQueue(20) {
	_threadCount = threadCount;
	for (uint32_t i = 0; i < threadCount; i++) {
		std::string name;
		string_format(name, "thread-%u", i);
		_threads.push_back(new Thread(this, name.c_str(), stackSize, priority, 0, MessageDispatcher::handleMailbox));
	}
}

void MessageDispatcher::attach(Mailbox& mailbox) {
//	_mailboxes.push_back(&mailbox);
}

void MessageDispatcher::attach(ActorCell& cell) {
	_actorCells.push_back(&cell);
}

void MessageDispatcher::detach(ActorCell& cell) {
	_actorCells.remove(&cell);
}

void MessageDispatcher::start() {
	for (Thread* thread : _threads) {
		thread->start();
	}
}

void MessageDispatcher::dispatchFromIsr(ActorCell& cell, Msg& msg) {
	cell.mailbox().enqueueFromIsr(msg);
	registerForExecution(&cell.mailbox(), true);
}

void MessageDispatcher::dispatch(ActorCell& cell, Msg& msg) {
	cell.mailbox().enqueue(msg);
	registerForExecution(&cell.mailbox(), true);
}
//_________ decide if mailbox should be wakened
//
void MessageDispatcher::registerForExecution(Mailbox* mbox,
        bool hasMessageHint) {
	if (mbox->canBeScheduledForExecution(hasMessageHint)) { //TODO
		if (mbox->setAsScheduled()) {
			DEBUG("'%s' scheduled", mbox->name());
			_workQueue.send(mbox, 100); // don't wait
		} else {
			DEBUG("'%s' already Scheduled ", mbox->name());
		}
	}
}

void MessageDispatcher::handleMailbox(void* thr) {

	Thread* thread = (Thread*) thr;
	MessageDispatcher& dispatcher = thread->dispatcher();
	NativeQueue<Mailbox*>& workQueue = dispatcher.workQueue();
	while (true) {
		Mailbox* mbox;
		while (workQueue.recv(&mbox, UINT32_MAX) != 0)
			;
		DEBUG(" mbox : 0x%X", mbox);
		DEBUG("'%s' mailbox recvd work.", mbox->name());
		myASSERT(mbox != 0);
		mbox->processMailbox(thread);
	}
}

NativeQueue<Mailbox*>& MessageDispatcher::workQueue() {
	return _workQueue;
}
