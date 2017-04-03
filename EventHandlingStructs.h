#ifndef EVENTHANDLINGSTRUCTS
#define EVENTHANDLINGSTRUCTS

#include <condition_variable>
#include <mutex>
#include <atomic>
#include "GuardedBuffer.h"

enum class HandlerReturnCall { Failed, Succeed};

template<class EventType>
struct Task {
	std::shared_ptr<EventType> Event;
	std::function<HandlerReturnCall(EventType&)> Function;
};

template<class EventType>
struct EventHandlersInfo {

	std::condition_variable th_Release;
	std::mutex th_ReleaseLock;

	GuardedBuffer<std::queue<Task<EventType>>>  TasksBuffer;

	std::atomic<unsigned> th_ThreadsRunning;
	std::atomic<unsigned> th_ThreadsAwaitingSynchronization;

	std::atomic<bool> CloseEventHandler;
};

#endif