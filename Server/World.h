#pragma once
#include "EventTimer.h"

class World
{
public:
	static World& instance();
	SOCKET& getServerSocket() { return server_socket; }
	HANDLE& getHandle() { return handle_iocp; }
	CEventTimer& getTimer();

	int getNewClientId();
	void disconnect(int c_id);

	void initializeTilemap();
	void initializeNpc();

	void processEvent(sEventData& avent);
	void addEvent(std::pair<int, int> id, COMP_TYPE type, int time);

	void acceptClient(OVER_EXP* ex_over, DWORD& num_bytes, ULONG_PTR& key);
	void recvClient(OVER_EXP* ex_over, DWORD& num_bytes, ULONG_PTR& key);
	void sendClient(OVER_EXP* ex_over, DWORD& num_bytes, ULONG_PTR& key);

	void moveNpcEvent(OVER_EXP* ex_over, DWORD& num_bytes, ULONG_PTR& key);
	void healEvent(OVER_EXP* ex_over, DWORD& num_bytes, ULONG_PTR& key);
	void monsterAttackEvent(OVER_EXP* ex_over, DWORD& num_bytes, ULONG_PTR& key);
	void AttackActiveEvent(OVER_EXP* ex_over, DWORD& num_bytes, ULONG_PTR& key);
	void npcRespawnEvent(OVER_EXP* ex_over, DWORD& num_bytes, ULONG_PTR& key);
	void activeSkiilCoolDownEvent(OVER_EXP* ex_over, DWORD& num_bytes, ULONG_PTR& key);
	void releaseSkillEvent(OVER_EXP* ex_over, DWORD& num_bytes, ULONG_PTR& key);

private:
	static World _instance;
	CEventTimer* _timer;
	SOCKET server_socket;
	HANDLE handle_iocp;
	std::mutex _lock;
};

