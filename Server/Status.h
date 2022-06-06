#pragma once

class CSession;

class CStatus
{
public:
	bool decreaseHp(CSession& client, int hp);
	void updateExp(CSession& client);
	void calculateMaxExp(CSession& client);
	void setLevelUp(CSession& client, int remainExp);
};

