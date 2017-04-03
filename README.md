# Cpp-Events-Handling

Couple header files that help to create event-driven applications in C++ 11.

IMPORTANT NOTES:

For every event you can add several handlers by AddHandler() function. 
When event is fired each handler is placed in the task queue and awaits to be handled by one of the threads. 
If number of threads is greater than 1, tasks are handled in no order.

In some cases the synchronization is needed. There are 4 similar functions to do so. 
Functions preceded by "extern" are ment to be executed by not-handling-events threads.
For handling-events threads "thread" functions are suitable.

To create own events (for example carrying data) you can use Evant as the base class.
You can also create your own base class, just keep in mind, it has to have at least 1 virtual function.
