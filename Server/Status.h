#pragma once

class CSession;

class CStatus
{
public:
	void healHp(CSession& client);
	bool decreaseHp(CSession& client, int hp);
	void updateExp(CSession& client, int mon_id);
	void calculateMaxExp(CSession& client);
	void setLevelUp(CSession& client, int remainExp);
};

