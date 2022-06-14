﻿#include "../Client/protocol.h"
#include "pch.h"
#include "Session.h"
#include "Astar.h"

using namespace std;
using namespace sf;
#pragma comment (lib, "WS2_32.LIB")
#pragma comment(lib, "MSWSock.lib")

const short SERVER_PORT = 4000;
const int BUFSIZE = 256;

SOCKET server_socket;
HANDLE handle_iocp;

int get_new_client_id()
{
	for (int i = 0; i < MAX_USER; ++i) {
		if (clients[i]._state == ST_FREE) {
			clients[i]._state = ST_ACCEPTED;
			return i;
		}
	}
	return -1;
}

void update_move_clients(int c_id, char& direction)
{
	short x = clients[c_id].x;
	short y = clients[c_id].y;
	switch (direction) {
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
	if (clients[c_id]._state == ST_FREE) {
		return;
	}
	closesocket(clients[c_id]._socket);
	clients[c_id]._state = ST_FREE;

	ChangeSector(c_id, false);
	update_database(c_id);

	for (auto& pl : clients) {
		if (pl._id == c_id) continue;
		if (pl._state != ST_INGAME) {
			continue;
		}
		pl.send_remove_object(c_id);
	}
}


void accept_client(OVER_EXP* ex_over, DWORD& num_bytes, ULONG_PTR& key);
void recv_client(OVER_EXP* ex_over, DWORD& num_bytes, ULONG_PTR& key);
void send_client(OVER_EXP* ex_over, DWORD& num_bytes, ULONG_PTR& key);


typedef void (*funcion_pointer)(OVER_EXP*, DWORD&, ULONG_PTR&);
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

		funcion_pointer fp[static_cast<int>(COMP_TYPE::length)]
			= { &accept_client, &recv_client,&send_client };
		(*fp[static_cast<int>(ex_over->_comp_type)])(ex_over, num_bytes, key);
	}
}


void accept_client(OVER_EXP* ex_over, DWORD& num_bytes, ULONG_PTR& key)
{
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
}

void recv_client(OVER_EXP* ex_over, DWORD& num_bytes, ULONG_PTR& key)
{
	if (0 == num_bytes) disconnect(key);
	int remain_data = num_bytes + clients[key]._prev_remain;

	clients[key].rebuild_packet(ex_over->_send_buf, remain_data);
	clients[key].do_recv();
}

void send_client(OVER_EXP* ex_over, DWORD& num_bytes, ULONG_PTR& key)
{
	if (0 == num_bytes) disconnect(key);
	delete ex_over;
}


void move_npc(int npc_id)
{
	unordered_set<int> old_vl;
	for (int i = 0; i < MAX_USER; ++i)
	{
		if (clients[i]._state != ST_INGAME) continue;
		if (distance(npc_id, i) <= RANGE) old_vl.insert(i);
	}

	//char dir = static_cast<char>(rand() % 4);
	//update_move_clients(npc_id,dir);

	CheckMoveSector(npc_id);

	unordered_set<int> new_vl;
	for (int i = 0; i < MAX_USER; ++i)
	{
		if (clients[i]._state != ST_INGAME) continue;
		if (distance(npc_id, i) <= RANGE) new_vl.insert(i);
	}

	for (auto p_id : new_vl) {
		clients[p_id].vl.lock();
		if (clients[p_id].view_list.count(npc_id) == 0) {
			clients[p_id].view_list.insert(npc_id);
			clients[p_id].vl.unlock();
			clients[p_id].send_add_object(npc_id);

				clients[npc_id]._target_id = p_id;
				cout << npc_id << ":find" << endl;
			
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

	if (clients[npc_id]._target_id != -1)
	{
		int target = clients[npc_id]._target_id;
		CAstar astar;
		pair<int, int> pos;

		clients[npc_id]._pathl.lock();
		astar.searchRoad(clients[npc_id].pathfind_pos, clients[npc_id].x, clients[npc_id].y, clients[target].x, clients[target].y);
		if (clients[npc_id].pathfind_pos.empty() == false)
		{
			pos = clients[npc_id].pathfind_pos.top();
			clients[npc_id].pathfind_pos.pop();

			clients[npc_id].x = pos.first;
			clients[npc_id].y = pos.second;
		}
		clients[npc_id]._pathl.unlock();
		cout << npc_id << ":chase - " << clients[npc_id].x <<" "<< clients[npc_id].y << endl;
		clients[target].send_move_packet(npc_id, 0);

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

void initialize_tilemap()
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
}

void initialize_npc()
{
	for (int i = 0; i < NUM_NPC + MAX_USER; ++i)
	{
		clients[i]._id = i;
	}

	for (int i = 0; i < NUM_NPC; ++i)
	{
		int npc_id = i + MAX_USER;
		clients[npc_id]._state = ST_INGAME;
		SetSector(i);
		sprintf_s(clients[npc_id]._name, "M-%d", npc_id);
	}
	cout << " load end .. " << endl;
}


int main()
{
	initialize_tilemap();
	initialize_npc();
	load_database();

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
