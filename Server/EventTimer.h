#pragma once
#include "pch.h"
#include <queue>

struct TIMER_EVENT {
	std::pair<int, int> id;
	COMP_TYPE ev;
	std::chrono::system_clock::time_point act_time;

	constexpr bool operator < (const TIMER_EVENT& _Left) const {
		return (act_time > _Left.act_time);
	}
};

class CEventTimer
{
public:
	void pushEvent(TIMER_EVENT ev);
	void update();
	bool isEventStart();
private:
	std::priority_queue<TIMER_EVENT> eventQueue;
	std::mutex timer_lock;
};

