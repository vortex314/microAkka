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
const char* cloneString(const char* s) {
	char* ps = new char[strlen(s) + 1];
	strcpy(ps, s);
	return (const char*) ps;
}

//_______________________________________________ Label
//
std::unordered_map<uid_type, Label::LabelStruct*>* Label::_labels;

std::unordered_map<uid_type, Label::LabelStruct*>* Label::labels() {
	if (_labels == 0) {
		_labels = new std::unordered_map<uid_type, LabelStruct*>();
	}
	return _labels;
}

Label::Label(const char* label)
		: Label(H(label), label) {
}

Label::Label(uid_type id) : Label(id,"unknown"){}

Label::Label() : Label(H("unknown")){

}

bool Label::operator==(Label& other){
	return other._pl->_uid == _pl->_uid;
}

uid_type Label::id() {
	return _pl->_uid;
}
const char* Label::label() {
	return _pl->_label;
}

const char* Label::label(uid_type uid){
	std::unordered_map<uid_type, LabelStruct*>::const_iterator p =
			labels()->find(uid);
	if (p == labels()->end()) return "NO LABEL";
		return (p->second->_label);
}

//_______________________________________________ Ref
//
std::unordered_map<uid_type,Ref::RefStruct*> Ref::_refs;
 Ref Ref::NotFound(0);

Ref::~Ref() {

}

const char* Ref::label(){
	return _pr->_label.label();
}

uid_type Ref::id(){
	return _pr->_label.id();
}

Ref Ref::findRef(uid_type id) {
	if (_refs.find(id) == _refs.end()) return 0;
	return Ref(_refs.find(id)->second);
}

Label Ref::cls() {
	return _pr->_cls;
}

void* Ref::object() {
	return _pr->_object;
}

bool Ref::operator==(Ref& that){
	return this->_pr==that._pr;
}

//_________________________________________________ MsgClass

//_________________________________________________ Msg

Msg::Msg()
		: Xdr(12) {
//	INFO("ctor %X : [%d]",this,capacity());
	add(UD_CLS, (uid_type) 0);
	add(UD_SRC, (uid_type) 0);
	add(UD_DST, (uid_type) 0);
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
Msg::Msg(Label cls, Label src)
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
	const char* sRq = Label::label(rq);
	if (sRq != 0)
		s = sRq;
	else WARN(" no label %u ", rq);
	s += "Reply";
	dst(req.src());
	src(req.dst());
	cls(Label(s.c_str()).id());
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

std::string Msg::toString(){
	std::string result;
	result += Label::label(src());
	result+= " == ";
	result += Label::label(cls());
	result += "=> ";
	result += Label::label(dst());
	return result;
}

//_________________________________________________ Actor

MsgClass MsgClass::ReceiveTimeout() {
	static MsgClass m("ReceiveTimeout");
	return m;
}
MsgClass MsgClass::PoisonPill() {
	static MsgClass m("PoisonPill");
	return m;
}

MsgClass MsgClass::AnyClass() {
	static MsgClass m("AnyClass");
	return m;
}

MsgClass MsgClass::Properties() {
	static MsgClass m("properties");
	return m;
}

MsgClass MsgClass::PropertiesReply() {
	static MsgClass m("propertiesReply");
	return m;
}
std::list<Actor*> Actor::_actors;

Actor::Actor()
		: _context() {
}

Actor::~Actor() {
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
	WARN(" unhandled : '%s'=>'%s'=>'%s'", Label::label(msg.src()), Label::label(msg
			.cls()), Label::label(msg.dst()));
}

ActorRef& Actor::sender() {
	return _context->sender();
}

//Msg& Actor::txdMsg() { return context().dispatcher().txdMsg(); }

//_______________________________________________ ActorRef
//

ActorRef& ActorRef::NoSender() {
	return *ActorRef::lookup(Label("NoSender").id());
}

ActorRef::ActorRef(Label label)
		: Ref(label, this, "ActorRef") {
	INFO(" created ActorRef '%s' = %d", label.label(), label.id());
}

ActorRef::~ActorRef() {

}

bool ActorRef::operator==(ActorRef& v) {
	return id() == v.id();
}

const char* ActorRef::path() {
	return label();
}

/*ActorRef& ActorRef::NoSender() {
 static LocalActorRef ref("NoSender",Mailbox::object(Label("NoSender").id(),));
 return ref;
 }*/

//_______________________________________________ LocalActorRef
LocalActorRef::LocalActorRef(Label label, ActorSystem& system, Props& props,
		MessageDispatcher& dispatcher)
		: ActorRef(label), _cell(*new ActorCell(system, *this, dispatcher, props)) {

}

LocalActorRef::~LocalActorRef() {

}

ActorRef* ActorRef::lookup(uid_type uid) {
	Ref ref = Ref::findRef(uid);
	if ( ref==Ref::NotFound) return 0;
	assert( UID("ActorRef")==ref.cls().id());
	return (ActorRef*) ref.object();
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

void LocalActorRef::forward(Msg& msg, ActorContext& context) {

}

Mailbox& LocalActorRef::mailbox() {
	return _cell.mailbox();
}

ActorCell& LocalActorRef::cell() {
	return _cell;
}

//____________________________________________________________ ActorSelection

typedef std::function<void(Msg&)> MessageHandler;

//________________________________________________________ Mailbox
//

Mailbox::Mailbox(ActorCell& cell, uint32_t queueSize)
		: _cell(cell) {
	_queue = xQueueCreate(queueSize, sizeof(Msg*));
	_currentStatus = Open;
}

int Mailbox::enqueue(Msg& msg) {
//	INFO("enqueue : %s ",msg.toString().c_str());
	Msg* px = new Msg(msg.size());
	*px = msg;
	configASSERT(msg.src() != 0);
	configASSERT(msg.dst() != 0);
	BaseType_t rc = xQueueSend(_queue, &px, 10);
	if (rc != pdTRUE) {
		WARN("queue full %s", Label::label(_cell.self().id()));
		delete px;
		return ENOENT;
	}
	return 0;
}

int Mailbox::dequeue(Msg& msg, uint32_t time) {
	Msg* px;
	if (xQueueReceive(_queue, &px, pdMS_TO_TICKS(time)) != pdTRUE) {
		return ENOENT;
	}
	(Msg&) msg = *px;
	delete px;
	return 0;
}

bool Mailbox::hasMessages() {
	return uxQueueMessagesWaiting(_queue);
}
// for now supposing mailbox always open
bool Mailbox::canBeScheduledForExecution(bool hasMessagesHint) {
	return hasMessagesHint || hasMessages();
}
// check suspend counter, not used here in fact
bool Mailbox::shouldProcessMessage() {
	return (_currentStatus & shouldNotProcessMask) == 0;
}
//TODO if already scheduled==true return false, else set scheduled, indicates winning thread
bool Mailbox::setAsScheduled() {
	while (true) {
		uint32_t s = _currentStatus;
		if ((s & shouldScheduleMask) != Open) return false;
		if (updateStatus(s, s | Scheduled)) return true;
	}
}
// set scheduled false
bool Mailbox::setAsIdle() {
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
	return std::atomic_compare_exchange_strong<uint32_t>(&_currentStatus, &oldStatus, newStatus);
#endif
}

void Mailbox::processMailbox(Thread* thread) {
	if (shouldProcessMessage()) {
		while (dequeue(thread->rxd(), 0) == 0) {
			_cell.currentThread(thread);
			_cell.invoke(thread->rxd());
		}
		_cell.resetReceiveTimeout();
		setAsIdle();
	}
}

//_______________________________________________________ ActorSystem
//
ActorSystem::ActorSystem(Label label, MessageDispatcher& defaultDispatcher)
		: Ref(label, this, "ActorSystem"), _defaultDispatcher(defaultDispatcher) {
	Label(AKKA_DST);
	Label(AKKA_SRC);
	Label(AKKA_CLS);
	Label(AKKA_ID);
	Label(AKKA_TIMER);
	_defaultDispatcher.start();
}

/*ActorRef& ActorSystem::actorFor(const char* address) {
 // TODO check local or remote
 ActorRef* ref = ActorRef::lookup(Label::add(address));
 if (ref == 0)
 ref = new ActorRef(address, &_defaultProps.mailbox());
 return *ref;
 }*/

ActorRef* ActorSystem::create(Actor* actor, const char* name, Props& props) {
	std::string path = label();
	path += "/";
	path += name;
	Label label(path.c_str());
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
	INFO(" new actor '%s'[%u] created", localActorRef->path(), localActorRef->id());
	_actorRefs.push_back(localActorRef);
//	props.dispatcher().attach(actorCell);
	return localActorRef;
}

MessageDispatcher& ActorSystem::defaultDispatcher() {
	return _defaultDispatcher;
}

std::list<ActorRef*>& ActorSystem::actorRefs() {
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
	if (_msgClass.id() == msg.cls() || _msgClass == MsgClass::AnyClass())
		return (_matcher(msg));
	return false;
}

std::string& Receiver::toString(std::string& s) {
	string_format(s, " class : %s  ", _msgClass.label());
	return s;
}
//_____________________________________________ Timer

void Timer::callBack(TimerHandle_t handle) {
	Timer* timer = (Timer*) pvTimerGetTimerID(handle);
//	INFO(" timer call back %s ",timer->label());
	timer->_timerScheduler.timerCallback(*timer);
}

Timer::Timer(Label key, bool autoReload, uint32_t interval, const Msg& m,
		TimerScheduler& scheduler)
		: Label(key), _timerScheduler(scheduler) {
	INFO("[%X] timer created %s : %u ", this, label(), interval);
	_msg = new Msg();
	*_msg = m;
	_msg->dst(scheduler.ref().id());
	_msg->src(scheduler.ref().id());
	_autoReload = autoReload;
	configASSERT((_timer = xTimerCreate(label(),pdMS_TO_TICKS(interval),autoReload,this,callBack))!=NULL);
	start();
}

void Timer::start() {
//	INFO("[%X] timer start",this);
	configASSERT(xTimerStart(_timer,0) == pdPASS);
}

void Timer::stop() {
//	INFO("[%X] timer stop",this);
	configASSERT(xTimerStop(_timer,0)==pdPASS);
}

Timer::~Timer() {
	INFO("[%X] timer dtor", this);
	xTimerDelete(_timer, 0);
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
	INFO("[%X] timer interval(%u)", this, interv);
	configASSERT(xTimerDelete(_timer,10)==pdPASS);
	configASSERT((_timer = xTimerCreate(label(),pdMS_TO_TICKS(interv),_autoReload,this,callBack))!=NULL);
	start();
}

Label Timer::key() {
	return id();
}

//__________________________________________________________ TimerScheduler

void TimerScheduler::timerCallback(Timer& timer) {
//	INFO(" timer call back : %s %s ",timer.label(),_ref.label());
	_ref.tell(timer.msg());
}

TimerScheduler::TimerScheduler(ActorRef& ref)
		: _ref(ref) {
}

Timer* TimerScheduler::find(Label key) {
	for (Timer* timer : _timers) {
		if (timer->key() == key) return timer;
	}
	return 0;
}

Label TimerScheduler::startPeriodicTimer(Label key, const Msg& msg, uint32_t msec) {
	Timer* timer = find(key.id());

	if (timer == 0) {
		_timers.push_back(new Timer(key, true, msec, msg, *this));
	} else {
		timer->msg(msg);
		timer->start();
	}
	return key.id();
}

Label TimerScheduler::startSingleTimer(Label key, const Msg& msg, uint32_t msec) {
	Timer* timer = find(key.id());

	if (timer == 0) {
		_timers.push_back(new Timer(key, false, msec, msg, *this));
	} else {
		timer->msg(msg);
		timer->start();
	}
	return key.id();
}

void TimerScheduler::cancel(Label key) {
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
		: _mailbox(*new Mailbox(*this, 10)), _dispatcher(dispatcher), _system(system), _self(ref) {
	_enable = true;
	_currentThread = 0;
	_timers = 0;
	_lastReceive = UINT64_MAX - 2 * UINT32_MAX;
	_inactivityPeriod = UINT32_MAX;
	_receive = 0;
	_prevReceive = 0;
	_actorCells.push_back(this);
	_semaphore = xSemaphoreCreateBinary();
	xSemaphoreGive(_semaphore);
	_actor = 0;
//	INFO(" ActorCell created %s [%d]",_self.path(),sizeof(ActorCell));
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
;

void ActorCell::invoke(Msg& msg) {
	while (xSemaphoreTake(_semaphore, (TickType_t)10) != pdTRUE) {
		printf(" xSemaphoreTake()  timed out ");
	}
	_receive->receive(*this, msg);
	if (xSemaphoreGive(_semaphore) != pdTRUE) {
		printf("xSemaphoreGive() failed");
	}
}

void ActorCell::sendMessage(Msg& msg) {
	_dispatcher.dispatch(*this, msg);
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
}

uint32_t ActorCell::receiveTimeout() {
	return _inactivityPeriod;
}

void ActorCell::resetReceiveTimeout() {
	_lastReceive = Sys::millis();
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
		uint32_t stackSize, uint32_t priority)
		: Ref(name, this, "Thread"), _dispatcher(dispatcher) {
	_stackSize = stackSize;
	_priority = priority;
	_task = 0;
}

void Thread::start(ThreadCode code) {
	BaseType_t rc = xTaskCreate(code, label(), _stackSize, this,
	tskIDLE_PRIORITY + 2, &_task);
	assert(rc==pdPASS);

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
		uint32_t priority) {
	_workQueue = xQueueCreate(20, sizeof(Mailbox*));
	_threadCount = threadCount;
	for (uint32_t i = 0; i < threadCount; i++) {
		std::string name;
		string_format(name, "thread-%u", i);
		_threads.push_back(new Thread(this, name.c_str(), stackSize, priority));
	}
	_unhandledCell = 0;

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

void MessageDispatcher::unhandled(ActorCell* cell) {
	_unhandledCell = cell;
}

void MessageDispatcher::start() {
	for (Thread* thread : _threads) {
		thread->start(MessageDispatcher::handleMailbox);
	}
}

void MessageDispatcher::dispatch(ActorCell& cell, Msg& msg) {
	cell.mailbox().enqueue(msg);
	registerForExecution(&cell.mailbox());
}
//_________ decide if mailbox should be wakened
//
void MessageDispatcher::registerForExecution(Mailbox* mbox) {
	if (mbox->canBeScheduledForExecution(true) ) { //TODO
		if ( mbox->setAsScheduled() ) {
		xQueueSend(_workQueue, &mbox, 0);
		}
	}
}

extern void* pxCurrentTCB;

void MessageDispatcher::handleMailbox(void* thr) {

	Thread* thread = (Thread*) thr;
	MessageDispatcher& dispatcher = thread->dispatcher();
	QueueHandle_t workQueue = dispatcher.workQueue();
	INFO("Thread : % s [%X]  ", thread->label(), pxCurrentTCB);
	while (true) {
		Mailbox* mbox;
		configASSERT(workQueue!=0);
		while (xQueueReceive(workQueue, &mbox, UINT32_MAX) != pdTRUE)
			;
		configASSERT(mbox!=0);

		mbox->processMailbox(thread);
	}
}

QueueHandle_t MessageDispatcher::workQueue() {
	return _workQueue;
}
