#pragma once
#include <queue>

struct TIMER_EVENT {
	pair<int, int> id;
	COMP_TYPE ev;
	chrono::system_clock::time_point act_time;

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
	priority_queue<TIMER_EVENT> eventQueue;
	mutex timer_lock;
};

