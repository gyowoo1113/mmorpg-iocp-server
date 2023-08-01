#pragma once
#include "pch.h"
#include <queue>

struct sEventData 
{
	std::pair<int, int> _id;
	COMP_TYPE _eventType;
	std::chrono::system_clock::time_point _eventStartTime;

	constexpr bool operator < (const sEventData& _Left) const {
		return (_eventStartTime > _Left._eventStartTime);
	}
};

class CEventTimer
{
public:
	void pushEvent(sEventData ev);
	void update();
	bool isEventStart();
private:
	std::priority_queue<sEventData> eventQueue;
	std::mutex timer_lock;
};

