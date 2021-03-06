#include <Native.h>

#if defined( __freeRTOS__ ) || defined( ESP_OPEN_RTOS ) || defined(ESP32_IDF)

#include <FreeRTOS.h>
#include <semphr.h>
#include <queue.h>
#include <task.h>
#include <timers.h>

template <typename T>
NativeQueue<T>::NativeQueue(uint32_t queueSize) {
	_queue = xQueueCreate(queueSize,sizeof(T));
}

template <typename T>
int NativeQueue<T>::send(T item,uint32_t msecWait) {
	BaseType_t rc = xQueueSend(_queue, &item, pdMS_TO_TICKS(msecWait) );
	if ( rc == pdTRUE ) return 0;
	return ENOSPC;
}

template <typename T>
int NativeQueue<T>::sendFromIsr(T item) {
	BaseType_t higherPriorityTaskWoken;
	BaseType_t rc = xQueueSendToFrontFromISR(_queue, &item, &higherPriorityTaskWoken );
	if ( rc == pdTRUE ) return 0;
	return ENOSPC;
}


template <typename T>
int NativeQueue<T>::recv(T* item,uint32_t msecWait) {
	BaseType_t rc = xQueueReceive(_queue, item, pdMS_TO_TICKS(msecWait));
	if ( rc == pdTRUE) return 0;
	return ENOENT;
}

template <typename T>
uint32_t NativeQueue<T>::messageCount() {
	return uxQueueMessagesWaiting(_queue);
}

typedef void (*TimerCallback)(void*);

void NativeTimer::freeRTOSCallback(TimerHandle_t handle) {
	NativeTimer* me = (NativeTimer*) pvTimerGetTimerID(handle);
	me->_callbackFunction(me->_callbackArg);
}

NativeTimer::NativeTimer(const char* name,bool autoReload, uint32_t interval,void* callbackArg,TimerCallback callbackFunction) :
	_callbackFunction(callbackFunction),
	_callbackArg(callbackArg),
	_autoReload(autoReload),
	_interval(interval) {
	configASSERT((_timer = xTimerCreate(name,pdMS_TO_TICKS(interval),autoReload,this,freeRTOSCallback))!=NULL);
}

NativeTimer::~NativeTimer() {
}

void NativeTimer::start() {
	configASSERT(xTimerStart(_timer,2) == pdPASS);
}

void NativeTimer::stop() {
	configASSERT(xTimerStop(_timer,2)==pdPASS);
}

void NativeTimer::reset() {
	configASSERT(xTimerReset(_timer,2)==pdPASS);
}

void NativeTimer::interval(uint32_t v) {
	configASSERT(xTimerChangePeriod(_timer,pdMS_TO_TICKS(v),10)==pdPASS);
}

typedef void(*TaskFunction)(void*);

NativeThread::NativeThread(const char* name,uint32_t stackSize,uint32_t priority,void* taskArg,TaskFunction taskFunction) {
	_name=name;
	_stackSize=stackSize;
	_priority=priority;
	_taskFunction = taskFunction;
	_taskArg=taskArg;
	_task=0;
}
void NativeThread::start() {
	BaseType_t rc = xTaskCreate(_taskFunction,_name, _stackSize, _taskArg,_priority, &_task);
	myASSERT(rc==pdPASS);
}
void NativeThread::wait() {

}

#endif

#if defined( __linux__ ) || defined( __APPLE__ )

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#ifndef __APPLE__
#include <sys/msg.h>
#endif
#include <sys/types.h>
#include <errno.h>
#include <Native.h>

//_______________________________________________________________________________
//

//________________________________________________________________________________
//

uint32_t NativeTimer::_idCounter = 0;

NativeThread::NativeThread(const char* name, uint32_t stackSize,
                           uint32_t priority, void* taskArg, TaskFunction taskFunction) {
	_name = name;
	_stackSize = stackSize;
	_priority = priority;
	_taskFunction = taskFunction;
	_taskArg = taskArg;
	_thread = 0;
}

void NativeThread::start() {
	myASSERT(
	    pthread_create(&_thread,NULL,(PthreadFunction)_taskFunction,_taskArg)==0);
#ifndef __APPLE__
	pthread_setname_np(_thread,_name.c_str());
#endif
}

void NativeThread::wait() {
	myASSERT(pthread_join(_thread,NULL)==0);
}

//________________________________________________________________________________
//

NativeTimerThread timerThread;

NativeTimer::NativeTimer(const char* name, bool autoReload, uint32_t interval,
                         void* callbackArg, TimerCallback callbackFunction) :
	_callbackFunction(callbackFunction), _callbackArg(callbackArg), _autoReload(
	    autoReload), _interval(interval) {
//	_timer = 0;
	_id = _idCounter++;
}

NativeTimer::~NativeTimer() {
}

void NativeTimer::start() {

	timerThread.addTimer(this);
}

void NativeTimer::stop() {
//	timerThread.removeTimer(this);
}

void NativeTimer::reset() {
}

void NativeTimer::interval(uint32_t v) {
	INFO("[%X] timer interval(%u)", this, v);
	_interval = v;
}

uint32_t NativeTimer::interval() {
	return _interval;
}

bool NativeTimer::autoReload() {
	return _autoReload;
}

void NativeTimer::invoke() {
	_callbackFunction(_callbackArg);
}

void NativeTimerThread::run() {
	while (true) {
		std::unique_lock<std::mutex> lock(m_Mutex);

		while (!m_Stop && m_Timers.empty()) {
			m_Condition.wait(lock);
		}

		if (m_Stop) {
			return;
		}

		if (m_Sort) {
			//Sort could be done at insert
			//but probabily this thread has time to do
			std::sort(m_Timers.begin(), m_Timers.end(),
			[](const NativeTimer * ti1, const NativeTimer * ti2) {
				return ti1->_timePoint > ti2->_timePoint;
			});
			m_Sort = false;
		}

		auto now = Clock::now();
		auto expire = m_Timers.back()->_timePoint;

		if (expire > now) { //can I take a nap?
			auto napTime = expire - now;
			m_Condition.wait_for(lock, napTime);

			//check again
			auto expire = m_Timers.back()->_timePoint;
			auto now = Clock::now();

			if (expire <= now) {
//				INFO(" invoke timer ");
				m_Timers.back()->invoke();
				auto t = m_Timers.back();
				m_Timers.pop_back();
				if (t->autoReload()) {
					t->_timePoint = Clock::now()
					                + std::chrono::milliseconds(t->interval());
					m_Timers.emplace_back(t);
					m_Sort = true;
				} else {
//					delete t;
				}
			}
		} else {
//			INFO(" invoke timer ");
			m_Timers.back()->invoke();
			auto t = m_Timers.back();
			m_Timers.pop_back();
			if (t->autoReload()) {
				t->_timePoint = Clock::now()
				                + std::chrono::milliseconds(t->interval());
				m_Timers.emplace_back(t);
				m_Sort = true;
			} else {
//				delete t;
			}
		}
	}
}

void NativeTimerThread::runStatic(void* th) {
	NativeTimerThread* me = (NativeTimerThread*) th;
	me->run();
}
NativeTimerThread::NativeTimerThread() :
	NativeThread("timerThread", 1024, 5, this, runStatic), m_Stop(false), m_Sort(
	    false) {
	start();

}
NativeTimerThread::~NativeTimerThread() {
	m_Stop = true;
	m_Condition.notify_all();
	this->wait();
}
void NativeTimerThread::addTimer(NativeTimer* timer) {
	{
		std::unique_lock<std::mutex> lock(timerThread.m_Mutex);
		timer->_timePoint = Clock::now()
		                    + std::chrono::milliseconds(timer->interval());
		timerThread.m_Timers.emplace_back(timer);
		timerThread.m_Sort = true;
	}
	// wake up
	timerThread.m_Condition.notify_one();
}

void NativeTimerThread::removeTimer(NativeTimer* timer) {
	{
		std::unique_lock<std::mutex> lock(timerThread.m_Mutex);
		for (auto t = m_Timers.begin(); t != m_Timers.end(); t++) {
			if (*t == timer)
				m_Timers.erase(t);
		}
		timerThread.m_Sort = true;
	}
	// wake up
	timerThread.m_Condition.notify_one();
}

#endif
