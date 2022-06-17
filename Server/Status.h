#pragma once

class CSession;

class CStatus
{
public:
	bool healHp(CSession& client);
	void checkHealEvent(CSession& client);
	bool decreaseHp(CSession& client, int hp);
	void updateExp(CSession& client, int mon_id);
	void calculateMaxExp(CSession& client);
	void setLevelUp(CSession& client, int remainExp);
	void respawn(CSession& client);
};

