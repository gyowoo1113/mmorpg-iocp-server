#include "../Client/protocol.h"
#include "pch.h"
#include "Session.h"

using namespace std;
using namespace sf;
#pragma comment (lib, "WS2_32.LIB")
#pragma comment(lib, "MSWSock.lib")

const short SERVER_PORT = 4000;
const int BUFSIZE = 256;

SOCKET server_socket;
HANDLE handle_iocp;

int tiles[W_WIDTH][W_HEIGHT] = {};

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

		update_move_clients(c_id, p->direction);
		CheckMoveSector(c_id);

		unordered_set<int> new_nl;
		new_nl = MakeNearList(c_id);

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
	if (c_id >= MAX_USER) return; 
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

	clients[n].vl.lock();
	if (clients[n].view_list.count(c_id))
	{
		clients[n].send_move_packet(c_id, p->client_time);
		clients[n].vl.unlock();
	}
	else
	{
		// 상대 view_list에 추가
		clients[n].view_list.insert(c_id);
		clients[n].vl.unlock();

		// 상대 => put_pl (나)
		clients[n].send_add_object(c_id);
	}
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
	unordered_set<int> old_vl;
	for (int i = 0; i < MAX_USER; ++i)
	{
		if (clients[i]._s_state != ST_INGAME) continue;
		if (distance(npc_id, i) <= RANGE) old_vl.insert(i);
	}

	char dir = static_cast<char>(rand() % 4);
	update_move_clients(npc_id,dir);

	CheckMoveSector(npc_id);

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
	for (int i = 0; i < NUM_NPC + MAX_USER; ++i)
	{
		clients[i]._id = i;
	}

	for (int i = 0; i < NUM_NPC; ++i)
	{
		int npc_id = i + MAX_USER;
		clients[npc_id]._s_state = ST_INGAME;
		SetSector(i);
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