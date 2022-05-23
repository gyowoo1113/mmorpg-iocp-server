#include <SFML/Graphics.hpp>

#include <iostream>
#include <WS2tcpip.h>
#include <MSWSock.h>

#include <thread>
#include <array>
#include <vector>
#include <mutex>
#include <string>
#include <unordered_set>
#include <fstream>
#include "../Client/protocol.h"
using namespace std;
using namespace sf;
#pragma comment (lib, "WS2_32.LIB")
#pragma comment(lib, "MSWSock.lib")

const short SERVER_PORT = 4000;
const int BUFSIZE = 256;
constexpr int RANGE = 5;

void SetSector(int id);
void CheckMoveSector(int id);
void ChangeSector(int id, bool update = true);
int distance(int a, int b);

class SESSION;

SOCKET server_socket;
HANDLE handle_iocp;

unordered_set<int> sector[W_WIDTH / 10][W_HEIGHT / 10];
int tiles[W_WIDTH][W_HEIGHT] = {};
mutex secl;

enum COMP_TYPE { OP_ACCEPT, OP_RECV, OP_SEND };
class OVER_EXP {
public:
	WSAOVERLAPPED _over;
	WSABUF _wsabuf;
	char _send_buf[BUF_SIZE];
	COMP_TYPE _comp_type;
	OVER_EXP()
	{
		_wsabuf.len = BUF_SIZE;
		_wsabuf.buf = _send_buf;
		_comp_type = OP_RECV;
		ZeroMemory(&_over, sizeof(_over));
	}
	OVER_EXP(char* packet)
	{
		_wsabuf.len = packet[0];
		_wsabuf.buf = _send_buf;
		ZeroMemory(&_over, sizeof(_over));
		_comp_type = OP_SEND;
		memcpy(_send_buf, packet, packet[0]);
	}
};

enum SESSION_STATE { ST_FREE, ST_ACCEPTED, ST_INGAME };

class SESSION {
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
	SESSION()
	{
		_id = -1;
		_socket = 0;
		x = rand() % W_WIDTH;
		y = rand() % W_HEIGHT;
		_name[0] = 0;
		_s_state = ST_FREE;
		_prev_remain = 0;
		_sector_x = x/10;
		_sector_y = y/10;
		x = rand() % W_WIDTH;
		y = rand() % W_HEIGHT;
		next_move_time = chrono::system_clock::now() + chrono::seconds(1);
	}
	~SESSION() {}

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

	unordered_set<int> MakeNearList();
};

array<SESSION, MAX_USER + NUM_NPC> clients;

int nearDirectionX[9] = { -1,-1,-1,0,0,0,1,1,1 };
int nearDirectionY[9] = { -1,0,1,-1,0,1,-1,0,1 };

unordered_set<int> SESSION::MakeNearList()
{
	int h = W_HEIGHT / 10;
	int w = W_WIDTH / 10;

	unordered_set<int> new_near_list;

	for (int i = 0; i < 9; ++i)
	{
		int dirX = _sector_x + nearDirectionX[i];
		int dirY = _sector_y + nearDirectionY[i];

		if (dirX < 0 || dirY < 0 || dirX > w - 1 || dirY > h - 1) continue;

		secl.lock();
		for (auto id : sector[dirX][dirY])
		{
			if (_id == id) continue;
			if (RANGE >= distance(_id, id))
			{
				new_near_list.insert(id);
			}
		}
		secl.unlock();

	}

	return (new_near_list);
}


int distance(int a, int b)
{
	return abs((clients[a].x - clients[b].x) + (clients[a].y - clients[b].y));
}

void SESSION::send_move_packet(int c_id, int client_time)
{
	SC_MOVE_PLAYER_PACKET p;
	p.id = c_id;
	p.size = sizeof(SC_MOVE_PLAYER_PACKET);
	p.type = SC_MOVE_PLAYER;
	p.x = clients[c_id].x;
	p.y = clients[c_id].y;
	p.client_time = client_time;
	do_send(&p);
}

void SESSION::send_add_object(int c_id)
{
	SC_ADD_PLAYER_PACKET p;
	p.id = c_id;
	p.size = sizeof(SC_ADD_PLAYER_PACKET);
	p.type = SC_ADD_PLAYER;
	p.x = clients[c_id].x;
	p.y = clients[c_id].y;
	strcpy_s(p.name, clients[c_id]._name);
	do_send(&p);
}

void SESSION::send_remove_object(int c_id)
{
	SC_REMOVE_PLAYER_PACKET p;
	p.id = c_id;
	p.size = sizeof(SC_REMOVE_PLAYER_PACKET);
	p.type = SC_REMOVE_PLAYER;
	do_send(&p);
}

void disconnect(int c_id);
void update_move_clients(int c_id, CS_MOVE_PACKET* p);
void check_view_list(const int& n, int& c_id, CS_MOVE_PACKET* p);
void remove_view_list(int c_id, int& view);
int get_new_client_id()
{
	for (int i = 0; i < MAX_USER; ++i) {
		clients[i]._sl.lock();
		if (clients[i]._s_state == ST_FREE) {
			clients[i]._s_state = ST_ACCEPTED;
			clients[i]._sl.unlock();
			return i;
		}
		clients[i]._sl.unlock();
	}
	return -1;
}

void process_packet(int c_id, char* packet)
{
	if (c_id < 0 || c_id > MAX_USER - 1) return;


	switch (packet[1]) {
	case CS_LOGIN: {
		CS_LOGIN_PACKET* p = reinterpret_cast<CS_LOGIN_PACKET*>(packet);
		clients[c_id]._sl.lock();
		if (clients[c_id]._s_state == ST_FREE) {
			clients[c_id]._sl.unlock();
			break;
		}
		if (clients[c_id]._s_state == ST_INGAME) {
			clients[c_id]._sl.unlock();
			disconnect(c_id);
			break;
		}

		strcpy_s(clients[c_id]._name, p->name);
		clients[c_id].send_login_info_packet();
		clients[c_id]._s_state = ST_INGAME;
		clients[c_id]._sl.unlock();

		clients[c_id].x = rand() % W_WIDTH;
		clients[c_id].y = rand() % W_HEIGHT;
		
		clients[c_id]._secl.lock();
		SetSector(c_id);
		clients[c_id]._secl.unlock();

		for (int i = 0; i < MAX_USER; ++i) {
			auto& pl = clients[i];
			if (pl._id == c_id) continue;
			pl._sl.lock();
			if (ST_INGAME != pl._s_state) {
				pl._sl.unlock();
				continue;
			}
			if (RANGE >= distance(c_id, pl._id)) {
				pl.vl.lock();
				pl.view_list.insert(c_id);
				pl.vl.unlock();
				pl.send_add_object(c_id);
			}
			pl._sl.unlock();
		}
		
		for (auto& pl : clients) {
			if (pl._id == c_id) continue;
			lock_guard<mutex> aa{ pl._sl };
			if (ST_INGAME != pl._s_state) continue;

			if (RANGE >= distance(pl._id, c_id)) {
				clients[c_id].vl.lock();
				clients[c_id].view_list.insert(pl._id);
				clients[c_id].vl.unlock();
				clients[c_id].send_add_object(pl._id);
			}
		}


		break;
	}
	case CS_MOVE: {
		CS_MOVE_PACKET* p = reinterpret_cast<CS_MOVE_PACKET*>(packet);

		update_move_clients(c_id, p);
		CheckMoveSector(c_id);

		unordered_set<int> new_nl;
		new_nl = clients[c_id].MakeNearList();

		clients[c_id].send_move_packet(c_id, p->client_time);

		for (auto n : new_nl)
		{
			if (clients[n]._id == c_id) continue;
			lock_guard<mutex> aa{ clients[n]._sl };
			if (ST_INGAME != clients[n]._s_state) continue;

			// view list에 없으면
			clients[c_id].vl.lock();
			if (clients[c_id].view_list.count(n) == 0)
			{
				//viewlist에 추가
				clients[c_id].view_list.insert(n);
				clients[c_id].vl.unlock();

				// 나 <= 상대 put
				clients[c_id].send_add_object(n);

				check_view_list(n, c_id, p);

			}
			else
			{
				clients[c_id].vl.unlock();
				check_view_list(n, c_id, p);
			}
		}

		//////////

		clients[c_id].vl.lock();
		unordered_set<int> new_list = clients[c_id].view_list;
		clients[c_id].vl.unlock();

		// view_list에 있는 모든 객체에 대해
		for (auto view : new_list)
		{
			// near에 없으면
			if (new_nl.count(view) == 0)
			{
				clients[c_id].vl.lock();
				clients[c_id].view_list.erase(view);
				clients[c_id].vl.unlock();
				remove_view_list(c_id, view);

				// 상대 view_list에 있으면
				clients[view].vl.lock();
				if (clients[view].view_list.count(c_id))
				{
					clients[view].view_list.erase(c_id);
					clients[view].vl.unlock();
					remove_view_list(view, c_id);
				}
				else
				{
					clients[view].vl.unlock();
				}
			}
		}
		break;
	}
	}
}

void remove_view_list(int c_id, int& view)
{
	if (c_id == view) return;
	clients[c_id]._sl.lock();
	if (clients[c_id]._s_state != ST_INGAME) {
		clients[c_id]._sl.unlock();
		return;
	}

	SC_REMOVE_PLAYER_PACKET p;
	p.id = view;
	p.size = sizeof(p);
	p.type = SC_REMOVE_PLAYER;
	clients[c_id].do_send(&p);
	clients[c_id]._sl.unlock();
}

void check_view_list(const int& n, int& c_id, CS_MOVE_PACKET* p)
{
	// 상대 view list에 있으면 / 없으면
	if (clients[n].view_list.count(c_id))
	{
		clients[n].send_move_packet(c_id, p->client_time);
	}
	else
	{
		// 상대 view_list에 추가
		clients[n].vl.lock();
		clients[n].view_list.insert(c_id);
		clients[n].vl.unlock();

		// 상대 => put_pl (나)
		SC_ADD_PLAYER_PACKET add_packet;
		add_packet.id = clients[c_id]._id;
		strcpy_s(add_packet.name, clients[c_id]._name);
		add_packet.size = sizeof(add_packet);
		add_packet.type = SC_ADD_PLAYER;
		add_packet.x = clients[c_id].x;
		add_packet.y = clients[c_id].y;
		clients[n].do_send(&add_packet);
	}
}

void update_move_clients(int c_id, CS_MOVE_PACKET* p)
{
	short x = clients[c_id].x;
	short y = clients[c_id].y;
	switch (p->direction) {
		case 0:
			if (y < W_HEIGHT - 1)
			{
				if (tiles[x][y + 1] == 0)
					y++;

			}
			break;
		
		case 1:
			if (y > 0)
			{
				if (tiles[x][y - 1] == 0)
					y--;
			}
			break;
		
		case 2:
			if (x > 0) 
			{
				if (tiles[x-1][y] == 0)
					x--;
			}
			break;
		
		case 3: 
			if (x < W_WIDTH - 1)
			{
				if (tiles[x + 1][y] == 0)
					x++;
			}
			break;
	}
	clients[c_id].x = x;
	clients[c_id].y = y;
}

void disconnect(int c_id)
{
	clients[c_id]._sl.lock();
	if (clients[c_id]._s_state == ST_FREE) {
		clients[c_id]._sl.unlock();
		return;
	}
	closesocket(clients[c_id]._socket);
	clients[c_id]._s_state = ST_FREE;
	clients[c_id]._sl.unlock();

	ChangeSector(c_id, false);

	for (auto& pl : clients) {
		if (pl._id == c_id) continue;
		pl._sl.lock();
		if (pl._s_state != ST_INGAME) {
			pl._sl.unlock();
			continue;
		}
		SC_REMOVE_PLAYER_PACKET p;
		p.id = c_id;
		p.size = sizeof(p);
		p.type = SC_REMOVE_PLAYER;
		pl.do_send(&p);
		pl._sl.unlock();
	}
}

void do_worker()
{
	while (true) {
		DWORD num_bytes;
		ULONG_PTR key;
		WSAOVERLAPPED* over = nullptr;
		BOOL ret = GetQueuedCompletionStatus(handle_iocp, &num_bytes, &key, &over, INFINITE);
		OVER_EXP* ex_over = reinterpret_cast<OVER_EXP*>(over);
		if (FALSE == ret) {
			if (ex_over->_comp_type == OP_ACCEPT) cout << "Accept Error";
			else {
				cout << "GQCS Error on client[" << key << "]\n";
				disconnect(static_cast<int>(key));
				if (ex_over->_comp_type == OP_SEND) delete ex_over;
				continue;
			}
		}

		switch (ex_over->_comp_type) {
		case OP_ACCEPT: {
			SOCKET c_socket = reinterpret_cast<SOCKET>(ex_over->_wsabuf.buf);
			int client_id = get_new_client_id();
			if (client_id != -1) {
				clients[client_id].x = 0;
				clients[client_id].y = 0;
				clients[client_id]._id = client_id;
				clients[client_id]._name[0] = 0;
				clients[client_id]._prev_remain = 0;
				clients[client_id]._socket = c_socket;
				CreateIoCompletionPort(reinterpret_cast<HANDLE>(c_socket), handle_iocp, client_id, 0);
				clients[client_id].do_recv();
				c_socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
			}
			else {
				cout << "Max user exceeded.\n";
			}
			ZeroMemory(&ex_over->_over, sizeof(ex_over->_over));
			ex_over->_wsabuf.buf = reinterpret_cast<CHAR*>(c_socket);
			int addr_size = sizeof(SOCKADDR_IN);
			AcceptEx(server_socket, c_socket, ex_over->_send_buf, 0, addr_size + 16, addr_size + 16, 0, &ex_over->_over);
			break;
		}
		case OP_RECV: {
			if (0 == num_bytes) disconnect(key);
			int remain_data = num_bytes + clients[key]._prev_remain;
			char* p = ex_over->_send_buf;
			while (remain_data > 0) {
				int packet_size = p[0];
				if (packet_size <= remain_data) {
					process_packet(static_cast<int>(key), p);
					p = p + packet_size;
					remain_data = remain_data - packet_size;
				}
				else break;
			}
			clients[key]._prev_remain = remain_data;
			if (remain_data > 0) {
				memcpy(ex_over->_send_buf, p, remain_data);
			}
			clients[key].do_recv();
			break;
		}
		case OP_SEND:
			if (0 == num_bytes) disconnect(key);
			delete ex_over;
			break;
		}
	}
}


void move_npc(int npc_id)
{
	short x = clients[npc_id].x;
	short y = clients[npc_id].y;
	unordered_set<int> old_vl;
	for (int i = 0; i < MAX_USER; ++i)
	{
		if (clients[i]._s_state != ST_INGAME) continue;
		if (distance(npc_id, i) <= RANGE) old_vl.insert(i);
	}


	switch (rand() % 4) {
	case 0: if (y > 0) y--; break;
	case 1: if (y < W_HEIGHT - 1) y++; break;
	case 2: if (x > 0) x--; break;
	case 3: if (x < W_WIDTH - 1) x++; break;
	}

	clients[npc_id].x = x;
	clients[npc_id].y = y;

	unordered_set<int> new_vl;
	for (int i = 0; i < MAX_USER; ++i)
	{
		if (clients[i]._s_state != ST_INGAME) continue;
		if (distance(npc_id, i) <= RANGE) new_vl.insert(i);
	}

	for (auto p_id : new_vl) {
		clients[p_id].vl.lock();
		if (clients[p_id].view_list.count(npc_id) == 0) {
			clients[p_id].view_list.insert(npc_id);
			clients[p_id].vl.unlock();
			clients[p_id].send_add_object(npc_id);
		}
		else {
			clients[p_id].send_move_packet(npc_id, 0);
			clients[p_id].vl.unlock();
		}
	}

	for (auto p_id : old_vl) {
		if (0 == new_vl.count(p_id)) {
			clients[p_id].vl.lock();
			if (clients[p_id].view_list.count(npc_id) == 1) {
				clients[p_id].view_list.erase(npc_id);
				clients[p_id].vl.unlock();
				clients[p_id].send_remove_object(npc_id);
			}
			else {
				clients[p_id].vl.unlock();
			}
		}
	}
}



void do_ai_ver_heart_beat()
{
	for (;; ) {
		auto start_t = chrono::system_clock::now();
		for (int i = 0; i < NUM_NPC; ++i) {
			int npc_id = i + MAX_USER;
			move_npc(npc_id);
		}
	}
}

void initialize_npc()
{
	for (int i = 0; i < NUM_NPC; ++i)
	{
		int npc_id = i + MAX_USER;
		sprintf_s(clients[npc_id]._name, "M-%d", npc_id);
	}
}


int main()
{
	ifstream in("../Resource/objects.txt");
	for (int i = 0; i < W_WIDTH; ++i)
	{
		for (int j = 0; j < W_HEIGHT; ++j)
		{
			char num;
			in >> num;
			int val = num - 'a';
			tiles[i][j] = (val == 0 || val > 6) ? 0 : 1;
		}
	}
	initialize_npc();

	WSADATA WSAData;
	WSAStartup(MAKEWORD(2, 2), &WSAData);
	server_socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);

	SOCKADDR_IN server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(SERVER_PORT);
	server_addr.sin_addr.S_un.S_addr = INADDR_ANY;
	bind(server_socket, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr));
	listen(server_socket, SOMAXCONN);

	SOCKADDR_IN cl_addr;
	int addr_size = sizeof(cl_addr);
	int client_id = 0;

	handle_iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);
	CreateIoCompletionPort(reinterpret_cast<HANDLE>(server_socket), handle_iocp, 9999, 0);
	SOCKET c_socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	OVER_EXP a_over;
	a_over._comp_type = OP_ACCEPT;
	a_over._wsabuf.buf = reinterpret_cast<CHAR*>(c_socket);
	AcceptEx(server_socket, c_socket, a_over._send_buf, 0, addr_size + 16, addr_size + 16, 0, &a_over._over);

	vector <thread> worker_threads;
	for (int i = 0; i < 6; ++i)
		worker_threads.emplace_back(do_worker);
	thread ai_thread{ do_ai_ver_heart_beat };
	ai_thread.join();
	for (auto& th : worker_threads)
		th.join();

	closesocket(server_socket);
	WSACleanup();
}

void SetSector(int id)
{
	int x = (clients[id].x) / 10;
	int y = (clients[id].y) / 10;

	secl.lock();
	sector[x][y].insert(id);
	secl.unlock();

	clients[id]._sector_x = x; 
	clients[id]._sector_y = y;
}

void CheckMoveSector(int id)
{
	int cl_x = clients[id].x;
	int cl_y = clients[id].y;
	int sec_x = clients[id]._sector_x;
	int sec_y = clients[id]._sector_y;

	if (cl_x != sec_x || cl_y != sec_y)
		ChangeSector(id);
}

void ChangeSector(int id, bool update)
{
	int sec_x = clients[id]._sector_x;
	int sec_y = clients[id]._sector_y;

	secl.lock();
	sector[sec_x][sec_y].erase(id);
	secl.unlock();
	if (update) SetSector(id);
}