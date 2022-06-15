#pragma once
#include"pch.h"
#include "over_exp.h"
#include "Status.h"
#include "SendPacket.h"

enum SESSION_STATE { ST_FREE, ST_ACCEPTED, ST_INGAME };

class CSession
{
	OVER_EXP _recv_over;
public:
	atomic<SESSION_STATE> _state = ST_FREE;

	int _id;
	SOCKET _socket;
	short x, y;
	char	_name[NAME_SIZE];
	int		_prev_remain;

	unordered_set<int> view_list;
	mutex vl;

	atomic<int> _target_id = -1;
	mutex _pathl;
	stack<pair<int, int>> pathfind_pos;

	short _sector_x, _sector_y;

	atomic<int> _hp = 100;
	atomic<int> _level = 1;
	atomic<int> _exp = 0;
	atomic<int> _maxExp = 0;

	chrono::system_clock::time_point next_move_time;

private:

	short monsterType = 0;
	short monsterMoveType = 0;

	CStatus _status;
	CSendPacket _sendPacket;

public:
	CSession();
	~CSession() {}

	void init(SOCKET& socket,int id);

	void do_recv()
	{
		DWORD recv_flag = 0;
		memset(&_recv_over._over, 0, sizeof(_recv_over._over));
		_recv_over._wsabuf.len = BUF_SIZE - _prev_remain;
		_recv_over._wsabuf.buf = _recv_over._send_buf + _prev_remain;
		WSARecv(_socket, &_recv_over._wsabuf, 1, 0, &recv_flag, &_recv_over._over, 0);
	}

	void do_send(void* packet)
	{
		OVER_EXP* sdata = new OVER_EXP{ reinterpret_cast<char*>(packet) };
		WSASend(_socket, &sdata->_wsabuf, 1, 0, 0, &sdata->_over, 0);
	}

	void rebuild_packet(char* send_buffer, int& remain_data);

	void send_move_packet(int c_id, int client_time);
	void send_add_object(int c_id);
	void send_remove_object(int c_id);
	void send_chat_packet(int c_id, const char* mess);
	void send_change_status_packet(int c_id);


	void update_move_view_list(CS_MOVE_PACKET* p, std::unordered_set<int>& new_nl);
	void check_erase_view_list(std::unordered_set<int>& new_nl);
	void check_view_list(int& c_id, CS_MOVE_PACKET* p);
	void remove_view_list(int& view);
	unordered_set<int> MakeNearList();

	void process_packet(char* packet);
	void moveObject(char* packet);
	void process_attack(char* packet);

	void chatSystemMessage(std::string& mess);

	bool decreaseHp(int hp);
	void heal();
	
	void setMonsterTypes();
	bool isMonsterMoving();
	short getMonsterType() { return monsterType; }
	
	
	friend class CStatus;
	friend class CSendPacket;
};

