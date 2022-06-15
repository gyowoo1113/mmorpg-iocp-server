#include "pch.h"
#include "EventTimer.h"

void CEventTimer::update()
{
	while (true)
	{
		lock_guard<mutex> tt{ timer_lock };
		if (isEventStart() == false) break;

		TIMER_EVENT ev = eventQueue.top();
		eventQueue.pop();

		World::instance().process_event(ev);
	}
}

bool CEventTimer::isEventStart()
{
	if (eventQueue.empty()) return false;
	if (eventQueue.top().act_time > chrono::system_clock::now()) return false;
	return true;
}

void CEventTimer::pushEvent(TIMER_EVENT ev)
{
	lock_guard<mutex> tt{ timer_lock };
	eventQueue.push(ev);
}