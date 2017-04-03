#ifndef EVENTHANDLINGTHREAD
#define EVENTHANDLINGTHREAD

#include <mutex>
#include "EventHandlingStructs.h"
#include <iostream>

template<class EventType>
void Run(EventHandlersInfo<EventType> & Info)
{
	try {
		while (!Info.CloseEventHandler)
		{
			std::unique_lock<std::mutex> _lock(Info.th_ReleaseLock);

			Info.th_Release.wait(_lock);
			_lock.unlock();

			++Info.th_ThreadsRunning;

			bool BufferIsNotEmpty = true;

			while (BufferIsNotEmpty)
			{
				auto Task = Info.TasksBuffer.SecurePopOne(BufferIsNotEmpty);

				if (BufferIsNotEmpty)
					if (Task.Function(*Task.Event) == HandlerReturnCall::Failed)
						throw std::runtime_error("Handler has failed to complete the task!");
			}

			--Info.th_ThreadsRunning;
		}
	}
	catch (std::exception E)
	{
		std::cerr << std::string("Unhandled exception has been thrown: ") + E.what();
	}
}

template<class EventType>
void CreateEventHandlingThread(EventHandlersInfo<EventType> & EventHandlersInfo)
{
	std::thread([&] {Run(EventHandlersInfo); }).detach();
}

#endif
