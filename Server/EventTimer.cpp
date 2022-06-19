#include "pch.h"
#include "EventTimer.h"

void CEventTimer::update()
{
	while (true)
	{
		std::lock_guard<std::mutex> tt{ timer_lock };
		if (isEventStart() == false) break;

		sEventData ev = eventQueue.top();
		eventQueue.pop();

		World::instance().processEvent(ev);
	}
}

bool CEventTimer::isEventStart()
{
	if (eventQueue.empty()) return false;
	if (eventQueue.top()._eventStartTime > std::chrono::system_clock::now()) return false;
	return true;
}

void CEventTimer::pushEvent(sEventData ev)
{
	eventQueue.emplace(ev);
}