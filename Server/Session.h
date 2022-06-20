#pragma once
#include"pch.h"
#include "over_exp.h"
#include "Status.h"
#include "SendPacket.h"
#include "Astar.h"

enum SESSION_STATE { ST_FREE, ST_ACCEPTED, ST_INGAME, ST_SLEEP };

class CSession
{
	OVER_EXP _recv_over;
public:
	std::atomic<SESSION_STATE> _state = ST_FREE;

	int		_id;
	SOCKET	_socket;
	short	 x, y;
	char	_name[NAME_SIZE];
	int		_prevRemainBuffer;

	std::unordered_set<int> view_list;
	std::mutex vl;

	std::atomic<int> _target_id = -1;
	std::mutex _pathl;
	std::stack<std::pair<int, int>> pathfind_pos;

	short _sector_x, _sector_y;

	std::atomic<int> _hp = 100;
	std::atomic<int> _level = 1;
	std::atomic<int> _exp = 0;
	std::atomic<int> _maxExp = 0;
	int _maxHp = 100;

	std::chrono::system_clock::time_point next_move_time;

private:

	short monsterType = 0;
	short monsterMoveType = 0;
	bool _isAttack = true;

	std::atomic<bool> _isShield = false;
	bool _isHealing = false;

	CAstar _astar;

	CStatus _status;
	CSendPacket _sendPacket;

public:
	CSession();
	~CSession() {}

	void init(SOCKET& socket,int id);

	void doRecv();
	void doSend(void* packet);

	void sendMovePacket(int c_id, int client_time);
	void sendAddObject(int c_id);
	void sendRemoveObject(int c_id);
	void sendChatPacket(int c_id, const char* mess);
	void sendChangeStatusPacket(int c_id);
	void sendAttackPacket(int c_id, int skill_type, short active_type = 0, short x = 0 , short y = 0);
	
	void sendMonsterAttack(int id, std::string& mess);


	void updateMoveViewList(int client_time, std::unordered_set<int>& new_nl);
	void checkEraseViewList(std::unordered_set<int>& new_nl);
	void checkViewList(int& c_id, int client_time);
	void removeViewList(int& view);
	void checkInsertViewList(int insert_id);
	std::unordered_set<int> MakeNearList();

	void rebuildPacket(char* send_buffer, int& remain_data);
	void processPacket(char* packet);
	void processAttack(char* packet);
	void setSkillCoolDown(CS_ATTACK_PACKET* p);
	void moveObject(char* packet);

	void chatMessage(std::string& mess, int id = -1);

	bool decreaseHp(int hp);
	void heal();
	void checkHealing();
	void setAttack(bool attack) { _isAttack = attack; }
	void respawnPlayer();
	void setRespawnStatus();
	void readyToRespawn();
	void releaseShield();

	bool getShield();
	
	void setMonsterTypes();
	short getMonsterType() { return monsterType; }
	void moveMonster();
	void movePathToNpc();
	void setPeaceTarget(int id);
	void setArgoTarget(int id);
	void checkTargetRelease(int id) {
		if (_target_id == id )_target_id = -1;
	}
	void checkArgoStart(int c_id);
	
	
	friend class CStatus;
	friend class CSendPacket;
};