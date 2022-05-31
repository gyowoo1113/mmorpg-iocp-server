#pragma once
#include"pch.h"
#include "over_exp.h"

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

	short _sector_x, _sector_y;

	atomic<int> _hp = 100;
	atomic<int> _level = 1;
	atomic<int> _exp = 0;
	atomic<int> _maxExp = 0;

	atomic<int> _target_id = -1;

	mutex _pathl;
	stack<pair<int, int>> pathfind_pos;
	
	chrono::system_clock::time_point next_move_time;
public:
	CSession()
	{
		_id = -1;
		_socket = 0;
		x = rand() % W_WIDTH;
		y = rand() % W_HEIGHT;
		_name[0] = 0;
		_state = ST_FREE;
		_prev_remain = 0;
		_sector_x = x / 10;
		_sector_y = y / 10;
		next_move_time = chrono::system_clock::now() + chrono::seconds(1);
		calculateMaxExp();
	}
	~CSession() {}

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

	void rebuild_packet(char* send_buffer, int& remain_data, const ULONG_PTR& key);

	void send_login_info_packet();
	void send_move_packet(int c_id, int client_time);
	void send_add_object(int c_id);
	void send_remove_object(int c_id);
	void send_chat_packet(int c_id, const char* mess);
	void send_change_status_packet(int c_id);


	void update_move_view_list(CS_MOVE_PACKET* p, std::unordered_set<int>& new_nl);
	void check_erase_view_list(std::unordered_set<int>& new_nl);
	void check_view_list(int& c_id, CS_MOVE_PACKET* p);
	void remove_view_list(int& view);

	void process_attack();

	bool decreaseHp(int hp);
	void updateExp(int c_id);
	void calculateMaxExp();
	void setLevelUp(int remainExp);
};

