#ifndef EVENTHANDLER
#define EVENTHANDLER

#include <map>
#include <functional>
#include <chrono>
#include <mutex>
#include <atomic>
#include <memory>
#include <typeindex>

#include "GuardedBuffer.h"
#include "EventHandlingStructs.h"
#include "EventHandlingThread.h"

template<class EventType, unsigned Threads = 4>
class EventHandler
{
public:
	template<class EventT>
	void Fire(EventT Event); // Fires new event of base type EventType.

	template<class EventT>
	void AddHandler(std::function<HandlerReturnCall(EventType&)> Handler); // Adds new handler for specified event type.

	void RemoveAllHandlers(); // Removes all handlers. Does not await any task;

	void extern_AwaitCurrent(); // Must be called from function not invoked by any event. Awaits all current tasks.
	void extern_AwaitAll(); // Must be called from function not invoked by any event. Awaits all current tasks and tasks invoked by them.

	void thread_AwaitCurrent(); // Must be called from function invoked by an event. Awaits all current tasks.
	void thread_AwaitAll(); // Must be called from function invoked by an event. Awaits all current tasks and tasks invoked by them.
	
	~EventHandler();
	EventHandler();
	
private:
	EventHandlersInfo<EventType> EventHandlersInfo;

	std::multimap<std::type_index, std::function<HandlerReturnCall(EventType&)>> Handlers;
	std::mutex HandlersMutex;

	void UnlockOne();
};


template<class EventType, unsigned Threads>
inline void EventHandler<EventType, Threads>::extern_AwaitCurrent()
{
	while (EventHandlersInfo.th_ThreadsRunning > EventHandlersInfo.th_ThreadsAwaitingSynchronization || !EventHandlersInfo.TasksBuffer.empty())
		std::this_thread::sleep_for(std::chrono::microseconds(1));
}

template<class EventType, unsigned Threads>
inline void EventHandler<EventType, Threads>::thread_AwaitCurrent()
{
	++EventHandlersInfo.th_ThreadsAwaitingSynchronization;

	while (EventHandlersInfo.th_ThreadsRunning > EventHandlersInfo.th_ThreadsAwaitingSynchronization || !EventHandlersInfo.TasksBuffer.empty())
		std::this_thread::sleep_for(std::chrono::microseconds(1));

	--EventHandlersInfo.th_ThreadsAwaitingSynchronization;
}

template<class EventType, unsigned Threads>
inline void EventHandler<EventType, Threads>::extern_AwaitAll()
{
	while (EventHandlersInfo.th_ThreadsRunning > 1 || !EventHandlersInfo.TasksBuffer.empty())
		std::this_thread::sleep_for(std::chrono::microseconds(1));
}

template<class EventType, unsigned Threads>
inline void EventHandler<EventType, Threads>::thread_AwaitAll()
{
	++EventHandlersInfo.th_ThreadsAwaitingSynchronization;

	while (EventHandlersInfo.th_ThreadsRunning > 1 || !EventHandlersInfo.TasksBuffer.empty())
		std::this_thread::sleep_for(std::chrono::microseconds(1));

	--EventHandlersInfo.th_ThreadsAwaitingSynchronization;
}

template<class EventType, unsigned Threads>
inline void EventHandler<EventType, Threads>::RemoveAllHandlers()
{
	std::lock_guard<std::mutex> _lock(HandlersMutex);

	Handlers.clear();
}

template<class EventType, unsigned Threads>
inline EventHandler<EventType, Threads>::~EventHandler()
{
	EventHandlersInfo.CloseEventHandler = true;
	extern_AwaitAll();
	EventHandlersInfo.th_Release.notify_all();
}

template<class EventType, unsigned Threads>
inline EventHandler<EventType, Threads>::EventHandler()
{
	for (int i = Threads; i--;)
	{
		CreateEventHandlingThread(EventHandlersInfo);
	}
};

template<class EventType, unsigned Threads>
inline void EventHandler<EventType, Threads>::UnlockOne()
{
	std::unique_lock<std::mutex> _lock(EventHandlersInfo.th_ReleaseLock);

	EventHandlersInfo.th_Release.notify_one();
}

template<class EventType, unsigned Threads>
template<class EventT>
inline void EventHandler<EventType, Threads>::Fire(EventT Event)
{
	std::shared_ptr<EventType> _Event(new EventT(Event));

	std::lock_guard<std::mutex> _lock(HandlersMutex);

	auto HandlersRange = Handlers.equal_range(Index<EventT>());

	for (auto i = HandlersRange.first; i != HandlersRange.second; ++i)
	{
		EventHandlersInfo.TasksBuffer.push({ _Event, i->second });
		UnlockOne();
	}
}

template<class EventType, unsigned Threads>
template<class EventT>
inline void EventHandler<EventType, Threads>::AddHandler(std::function<HandlerReturnCall(EventType&)> Handler)
{
	std::lock_guard<std::mutex> _lock(HandlersMutex);
	Handlers.insert(std::make_pair(Index<EventT>(), Handler));
}


#endif