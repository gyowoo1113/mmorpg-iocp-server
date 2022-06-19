#include "pch.h"
#include "EventTimer.h"

void CEventTimer::update()
{
	while (true)
	{
		std::lock_guard<std::mutex> tt{ timer_lock };
		if (isEventStart() == false) break;

		TIMER_EVENT ev = eventQueue.top();
		eventQueue.pop();

		World::instance().processEvent(ev);
	}
}

bool CEventTimer::isEventStart()
{
	if (eventQueue.empty()) return false;
	if (eventQueue.top().act_time > std::chrono::system_clock::now()) return false;
	return true;
}

void CEventTimer::pushEvent(TIMER_EVENT ev)
{
	eventQueue.emplace(ev);
}