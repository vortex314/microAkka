#include <Akka.h>

#include <thread>
#include <queue>
#include <unistd.h>

#include <pthread.h>
#include <list>
#include <iostream>
#include <string>
#include <mutex>
#include <condition_variable>
#include <algorithm>

//
//__________________________________________________ TIMERS _____________________________
//

class TimerDetail {
	public:
		TimerCallbackFunction_t _callback;
		void* _arg;
		bool _autoReload;
};

//sample
#include <iostream>
#include <string>

void TimerCall(TimerDetail* ptd) {
	std::cout << (uint64_t) ptd << std::endl;
}



template<class T>
class TimerThread {
	public:
		typedef std::chrono::high_resolution_clock clock_t;

		struct TimerInfo {
				clock_t::time_point m_TimePoint;
				T m_User;
				bool m_Active;
				TimerCallbackFunction_t _callback;
				void * _callbackArg;
				std::string _name;
				bool _autoReload;

				template<class TArg1>
				TimerInfo(clock_t::time_point tp, TArg1 && arg1)
						: m_TimePoint(tp),
						  m_User(std::forward<TArg1>(arg1)) {
						m_Active=false;
				}


		};

		std::unique_ptr<std::thread> m_Thread;
		std::vector<TimerInfo> m_Timers;
		std::mutex m_Mutex;
		std::condition_variable m_Condition;
		bool m_Stop;
		bool m_Sort;

		void TimerLoop() {
			for (;;) {
				std::unique_lock<std::mutex> lock(m_Mutex);

				while (!m_Stop && m_Timers.empty()) {
					m_Condition.wait(lock);
				}

				if (m_Stop) {
					return;
				}

				if (m_Sort) {
					//Sort could be done at insert
					//but probably this thread has time to do
					std::sort(m_Timers.begin(), m_Timers.end(), [](const TimerInfo & ti1, const TimerInfo & ti2)
					{
						return ti1.m_TimePoint > ti2.m_TimePoint;
					});
					m_Sort = false;
				}

				auto now = clock_t::now();
				auto expire = m_Timers.back().m_TimePoint;

				if (expire > now) //can I take a nap?
						{
					auto napTime = expire - now;
					m_Condition.wait_for(lock, napTime);

					//check again
					auto expire = m_Timers.back().m_TimePoint;
					auto now = clock_t::now();

					if (expire <= now) {
						TimerCall(m_Timers.back().m_User);
						m_Timers.pop_back();
					}
				} else {
					TimerCall(m_Timers.back().m_User);
					m_Timers.pop_back();
				}
			}
		}

		template<class TArg1>
		uint32_t CreateTimer(int ms, TArg1 && arg1) {
			uint32_t index;
			{
				std::unique_lock<std::mutex> lock(m_Mutex);
				index = m_Timers.size();
				m_Timers.emplace_back(TimerThread<T>::TimerInfo(TimerThread<T>::clock_t::now()
						+ std::chrono::milliseconds(ms), std::forward<TArg1>(arg1)));
				m_Sort = true;
			}
			// wake up
			m_Condition.notify_one();
			return index;
		}

		template<class TArg1, class TArg2>
		uint32_t CreateTimer(int ms, TArg1 && arg1, TArg2 && arg2) {
			uint32_t index;

			{
				std::unique_lock<std::mutex> lock(m_Mutex);
				index = m_Timers.size();
				m_Timers.emplace_back(TimerThread<T>::TimerInfo(TimerThread<T>::clock_t::now()
						+ std::chrono::milliseconds(ms), std::forward<TArg1>(arg1), std::forward<
						TArg2>(arg2)));
				m_Sort = true;
			}
			// wake up
			m_Condition.notify_one();
			return index;

		}

	public:
		TimerThread()
				: m_Stop(false), m_Sort(false) {
			m_Thread.reset(new std::thread(std::bind(&TimerThread::TimerLoop, this)));
		}

		~TimerThread() {
			m_Stop = true;
			m_Condition.notify_all();
			m_Thread->join();
		}
};

TimerThread<TimerDetail*>* timers = 0;
//_______________________________________________________________________
//
#include <time.h>
#include <signal.h>

class PosixTimer {
	public:
		uint32_t _interval;
		timer_t _posixHandle;
		TimerCallbackFunction_t _callback;
		void * _callbackArg;
		std::string _name;
		bool _autoReload;
		PosixTimer(const char* name, uint32_t interval, bool autoReload,
				TimerCallbackFunction_t func, void *arg) {
			_interval = interval;
			_name = name;
			_autoReload = autoReload;
			_callback = func;
			_callbackArg = arg;
			struct sigevent sigEvent;
			memset((void *)&sigEvent, 0, sizeof(sigEvent));

			sigEvent.sigev_notify = SIGEV_THREAD;
			sigEvent.sigev_notify_function = (void (*)(__sigval_t))func;//
			sigEvent.sigev_notify_attributes = NULL;//
			sigEvent.sigev_value.sival_ptr = arg;
			sigEvent.sigev_signo = SIGALRM;
			int rc = timer_create(CLOCK_REALTIME, &sigEvent, &_posixHandle);
			assert(rc == 0);
		}
		int start() {
			struct itimerspec tNew, tOld;
/*			tNew.it_value.tv_nsec = (xTicksToWait % 1000) * 1000000;
			tNew.it_value.tv_sec = (xTicksToWait / 1000);*/
			tNew.it_value.tv_nsec = 0;
			tNew.it_value.tv_sec = _interval / 1000;
			tNew.it_interval.tv_nsec = (_interval % 1000) * 1000000;
			tNew.it_interval.tv_sec = _interval / 1000;
			int flags = 0;
			int rc = timer_settime( _posixHandle, flags, &tNew, &tOld);
			assert(rc==0);
			return rc;
		}
};
//______________________________________________________________ xTimer
//
BaseType_t xTimerCreateTimerTask(void) PRIVILEGED_FUNCTION {
	timers = new TimerThread<TimerDetail*>();
	return pdPASS;
}

BaseType_t xTimerGenericCommand(TimerHandle_t xTimer,
		const BaseType_t xCommandID, const TickType_t xOptionalValue,
		BaseType_t * const pxHigherPriorityTaskWoken,
		const TickType_t xTicksToWait) PRIVILEGED_FUNCTION {
	if (xCommandID == tmrCOMMAND_START) {
		// xOptionalValue = start ticks
		// pxHigherPriorityTaskWoken==NULL
		// xTicksToWait : wait ticks
		PosixTimer* pt = (PosixTimer*)xTimer;
		if (pt->start()) return pdPASS;
		return pdFAIL;
	}
	return 0;
}
//_____________________________________________________________
//
void *pvTimerGetTimerID(TimerHandle_t xTimer) {
	PosixTimer* pt = (PosixTimer*)xTimer;
	return pt->_callbackArg;
}
//_____________________________________________________________
//
TimerHandle_t xTimerCreate(const char * const pcTimerName,
		const TickType_t xTimerPeriod, const UBaseType_t uxAutoReload,
		void * const pvTimerID, TimerCallbackFunction_t pxCallbackFunction) {

	return (TimerHandle_t) new PosixTimer(pcTimerName, xTimerPeriod, uxAutoReload, pxCallbackFunction, pvTimerID);
	/*	if ( timers==0 ) xTimerCreateTimerTask();
	 return (TimerHandle_t)timers->CreateTimer(2000, new TimerDetail {
	 (TimerCallbackFunction_t) pxCallbackFunction, (void*) pvTimerID,
	 uxAutoReload });*/
}
/*
 int mmain() {
 std::cout << "start" << std::endl;
 TimerThread<TimerDetail*> timers;

 timers.CreateTimer(2000, new TimerDetail {
 (TimerCallbackFunction_t) TimerCall, (void*) 0, true });
 //	timers.CreateTimer( 5000, 2);
 //	timers.CreateTimer( 100, 3);

 std::this_thread::sleep_for(std::chrono::seconds(5));
 std::cout << "end" << std::endl;
 return 0;
 }*/
//TimerThread<void*> timers;
//
//__________________________________________________ TASK _____________________________
//
//typedef int portBASE_TYPE;
uint32_t nada;
void* pxCurrentTCB = &nada;

//extern "C" {

static std::list<std::thread*> _threads;

typedef struct {
		char pcName[100];
		void *pvParameters;
		TaskFunction_t task;
} ThreadArg;

void initThread(ThreadArg* arg) {
	INFO(" starting thread %s", arg->pcName);
#ifdef __linux__
	pthread_setname_np(pthread_self(), arg->pcName);
#else
	pthread_setname_np(arg->pcName);
#endif
	return arg->task(arg->pvParameters);
}

BaseType_t xTaskCreate(TaskFunction_t pxTaskCode, const char * const pcName, /*lint !e971 Unqualified char types are allowed for strings and single characters only. */
const configSTACK_DEPTH_TYPE usStackDepth, void * const pvParameters,
		UBaseType_t uxPriority, TaskHandle_t * const pxCreatedTask)
				PRIVILEGED_FUNCTION {
	ThreadArg* arg = new ThreadArg();
	strcpy(arg->pcName, pcName);
	arg->pvParameters = pvParameters;
	arg->task = pxTaskCode;

	std::thread* thread = new std::thread(initThread, arg);
	_threads.push_back(thread);
	assert(thread!=NULL);
	return pdPASS;
}

void vTaskDelay(const TickType_t xTicksToDelay) PRIVILEGED_FUNCTION {
	usleep((uint32_t) xTicksToDelay * 10000);
}
void vTaskStartScheduler(void) PRIVILEGED_FUNCTION {
	for (std::thread* t : _threads) {
		t->join();
	}
}
//
//_______________________________________________________ QUEUE ____________
//

#include <queue>
#include <memory>
#include <mutex>
#include <condition_variable>
template<typename T>
class threadsafe_queue {
	private:
		mutable std::mutex mut;
		std::queue<T> data_queue;
		std::condition_variable data_cond;
	public:
		threadsafe_queue() {
		}
		threadsafe_queue(threadsafe_queue const& other) {
			std::lock_guard<std::mutex> lk(other.mut);
			data_queue = other.data_queue;
		}
		void push(T new_value) {
			std::lock_guard<std::mutex> lk(mut);
			data_queue.push(new_value);
			data_cond.notify_one();
		}
		void wait_and_pop(T& value) {
			std::unique_lock<std::mutex> lk(mut);
			data_cond.wait(lk, [this] {return !data_queue.empty();});
			value = data_queue.front();
			data_queue.pop();
		}
		std::shared_ptr<T> wait_and_pop() {
			std::unique_lock<std::mutex> lk(mut);
			data_cond.wait(lk, [this] {return !data_queue.empty();});
			std::shared_ptr<T> res(std::make_shared<T>(data_queue.front()));
			data_queue.pop();
			return res;
		}
		bool try_pop(T& value) {
			std::lock_guard<std::mutex> lk(mut);
			if (data_queue.empty()) return false;
			value = data_queue.front();
			data_queue.pop();
			return true;
		}
		std::shared_ptr<T> try_pop() {
			std::lock_guard<std::mutex> lk(mut);
			if (data_queue.empty()) return std::shared_ptr<T>();
			std::shared_ptr<T> res(std::make_shared<T>(data_queue.front()));
			data_queue.pop();
			return res;
		}
		bool empty() const {
			std::lock_guard<std::mutex> lk(mut);
			return data_queue.empty();
		}
};
UBaseType_t uxQueueMessagesWaiting(QueueHandle_t xQueue) {
	threadsafe_queue<void*>* queue = new threadsafe_queue<void*>();
	return !queue->empty();
}

QueueHandle_t xQueueGenericCreate(const UBaseType_t uxQueueLength,
		const UBaseType_t uxItemSize, const uint8_t ucQueueType) {
	threadsafe_queue<void*>* queue = new threadsafe_queue<void*>();
	return (QueueHandle_t) queue;
}
BaseType_t xQueueGenericSend(
xQueueHandle pxQueue, const void * const pvItemToQueue,
portTickType xTicksToWait, BaseType_t xCopyPosition) {
	threadsafe_queue<void*>* queue = (threadsafe_queue<void*>*) pxQueue;
	queue->push((void*) pvItemToQueue);
	return pdPASS;
}

BaseType_t xQueueGenericReceive( xQueueHandle xQueue, void * const pvBuffer,
portTickType xTicksToWait, BaseType_t xJustPeek) {
	threadsafe_queue<void*>* queue = (threadsafe_queue<void*>*) xQueue;
	void* v;
	queue->wait_and_pop(v);
	return pdPASS;
}

BaseType_t xQueueReceive(QueueHandle_t xQueue, void * const pvBuffer,
		TickType_t xTicksToWait) PRIVILEGED_FUNCTION {
	threadsafe_queue<void*>* queue = (threadsafe_queue<void*>*) xQueue;
	void* v;
	queue->wait_and_pop(v);
	return pdPASS;
}

TickType_t xTaskGetTickCount(void) {
	Sys::millis();
	return pdPASS;

}

//} // extern "C"

