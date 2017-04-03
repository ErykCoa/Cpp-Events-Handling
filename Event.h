#ifndef EVENT
#define EVENT

#include <exception>
#include <typeindex>
#include <chrono>

struct Event {
	virtual ~Event() {}
};

template<class EventName>
inline std::type_index Index()
{
	return std::type_index(typeid(EventName));
}

template<class EventName>
inline EventName & Unwrap(Event & Item)
{
	EventName &Res = dynamic_cast<EventName&>(Item);
	return Res;
}

#endif
