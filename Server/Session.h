#pragma once
#include"pch.h"
#include "over_exp.h"

enum SESSION_STATE { ST_FREE, ST_ACCEPTED, ST_INGAME };

class CSession
{
	OVER_EXP _recv_over;
public:
	mutex	_sl;
	SESSION_STATE _s_state;

	int _id;
	SOCKET _socket;
	short x, y;
	char	_name[NAME_SIZE];
	int		_prev_remain;

	unordered_set<int> view_list;
	mutex vl;

	short _sector_x, _sector_y;
	mutex _secl;

	chrono::system_clock::time_point next_move_time;
public:
	CSession()
	{
		_id = -1;
		_socket = 0;
		x = rand() % W_WIDTH;
		y = rand() % W_HEIGHT;
		_name[0] = 0;
		_s_state = ST_FREE;
		_prev_remain = 0;
		_sector_x = x / 10;
		_sector_y = y / 10;
		next_move_time = chrono::system_clock::now() + chrono::seconds(1);
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

	void send_login_info_packet()
	{
		SC_LOGIN_INFO_PACKET p;
		p.id = _id;
		p.size = sizeof(SC_LOGIN_INFO_PACKET);
		p.type = SC_LOGIN_INFO;
		p.x = x;
		p.y = y;
		do_send(&p);
	}

	void send_move_packet(int c_id, int client_time);
	void send_add_object(int c_id);
	void send_remove_object(int c_id);

	void update_move_view_list(CS_MOVE_PACKET* p, std::unordered_set<int>& new_nl);
	void check_erase_view_list(std::unordered_set<int>& new_nl);
};

