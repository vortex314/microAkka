/*
 * Native.h
 *
 *  Created on: Feb 9, 2019
 *      Author: lieven
 */

#ifndef SRC_NATIVE_H_
#define SRC_NATIVE_H_

#include <stdint.h>
#include <Log.h>

template <typename T>
class AbstractNativeQueue {
public:
	virtual ~AbstractNativeQueue(){};
	virtual int recv(T* item, uint32_t to)=0;
	virtual int send(T item, uint32_t to)=0;
	virtual int sendFromIsr(T item)=0;
	virtual uint32_t messageCount()=0;
};
typedef void (*TaskFunction)(void*);

class AbstractNativeThread {
public:
	virtual ~AbstractNativeThread(){};
	virtual void start()=0;
	virtual void wait()=0;
};

#if defined( __freeRTOS__ ) || defined( ESP_OPEN_RTOS ) || defined(ESP32_IDF)

#include <FreeRTOS.h>
#include <semphr.h>
#include <queue.h>
#include <task.h>
#include <timers.h>

template <typename T>
class NativeQueue : public AbstractNativeQueue<T> {
	QueueHandle_t _queue;
public:
	NativeQueue(uint32_t queueSize);
	int send(T item,uint32_t msecWait);
	int recv(T* item,uint32_t msecWait);
	int sendFromIsr(T item);
	uint32_t messageCount();
};

typedef void (*TimerCallback)(void*);

class NativeTimer {
	TimerHandle_t _timer;
	TimerCallback _callbackFunction;
	void* _callbackArg;
	bool _autoReload;
	uint32_t _interval;
public:
	static void freeRTOSCallback(TimerHandle_t handle);
	NativeTimer(const char* name,bool autoReload , uint32_t interval,void* callbackArg,TimerCallback callbackFunction);

	~NativeTimer();
	void start();
	void stop();
	void reset();
	void interval(uint32_t v);
};

typedef void(*TaskFunction)(void*);

class NativeThread {
	const char* _name;
	uint32_t _stackSize = 1024;
	uint32_t _priority = tskIDLE_PRIORITY + 1;
	TaskFunction _taskFunction;
	void* _taskArg;
	TaskHandle_t _task;
public:
	NativeThread(const char* name,uint32_t stackSize,uint32_t priority,void* taskArg,TaskFunction taskFunction);
	void start();
	void wait();
};

#endif
#if defined( __linux__ ) || defined(__APPLE__)

#ifndef __APPLE__
#include <sys/msg.h>
#endif

#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <errno.h>
//#include <signal.h>
#include <time.h>
#include <condition_variable>
#include <vector>
#include <algorithm>
#include <Log.h>
#include <functional>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>


template <typename T>
class NativeQueue :public AbstractNativeQueue<T> {
private:
	std::queue<T> queue_;
	std::mutex mutex_;
	std::condition_variable cond_;
public:
	NativeQueue(uint32_t queueSize) {};
	~NativeQueue() {} ;
	int recv(T* item, uint32_t to);
	int send(T item, uint32_t to);
	int sendFromIsr(T item);

	uint32_t messageCount();
//  Queue()=default;
	NativeQueue<T>(const NativeQueue<T>& other) = delete;
//  Queue& operator=(const Queue&) = delete; // disable assignment

};


template <typename T>
int NativeQueue<T>::recv(T* item, uint32_t to) {
	std::unique_lock<std::mutex> mlock(mutex_);
	std::chrono::milliseconds waitTime(to);
	if (queue_.empty()) {
		cond_.wait_for(mlock, waitTime);
		if (queue_.empty())
			return ENOENT;
	}
	*item = queue_.front();
	queue_.pop();
	return 0;
}
template <typename T>
int NativeQueue<T>::send(T item, uint32_t to) {
	std::unique_lock<std::mutex> mlock(mutex_);
	queue_.push(item);
	mlock.unlock();
	cond_.notify_one();
	return 0;
}
template <typename T>
int NativeQueue<T>::sendFromIsr(T item) {
	return 0; // NOT IMPLEMENTED ON LINUX
}
template <typename T>
uint32_t NativeQueue<T>::messageCount() {
	return queue_.size() ;
}


typedef void*(*PthreadFunction)(void*);
#define tskIDLE_PRIORITY 0

class NativeThread : public AbstractNativeThread{
	std::string _name;
	uint32_t _stackSize = 1024;
	uint32_t _priority = tskIDLE_PRIORITY + 1;
	TaskFunction _taskFunction;
	void* _taskArg;
	pthread_t _thread;
public:
	NativeThread(const char* name, uint32_t stackSize, uint32_t priority,
			void* taskArg, TaskFunction taskFunction);
	void start();
	void wait();
};

//#include <sort>

typedef void (*TimerCallback)(void*);
typedef std::chrono::milliseconds Interval;
typedef std::function<void(void)> Timeout;
typedef std::chrono::high_resolution_clock Clock;

class NativeTimer {
//	timer_t _timer;
	TimerCallback _callbackFunction;
	void* _callbackArg;
	bool _autoReload;
	uint32_t _interval;
	uint32_t _id;
	static uint32_t _idCounter;

public:
	std::chrono::high_resolution_clock::time_point _timePoint;

	NativeTimer(const char* name, bool autoReload, uint32_t interval,
			void* callbackArg, TimerCallback callbackFunction);

	~NativeTimer();
	void start();
	void stop();
	void reset();
	void interval(uint32_t v);
	uint32_t interval();
	bool autoReload();

	void invoke();

};
//___________________________________________________________________________
//
class NativeTimerThread: public NativeThread {
//		std::unique_ptr<std::thread> m_Thread;
	std::vector<NativeTimer*> m_Timers;
	std::mutex m_Mutex;
	std::condition_variable m_Condition;
	bool m_Stop;
	bool m_Sort;

public:

	void run();
	static void runStatic(void* th);
	NativeTimerThread();
	~NativeTimerThread();
	void addTimer(NativeTimer* timer);
	void removeTimer(NativeTimer* timer);

};

#endif

#endif /* SRC_NATIVE_H_ */
