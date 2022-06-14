#pragma once
#include "pch.h"
#include "World.h"

World World::_instance;

World& World::instance()
{
	return _instance;
}

// -------------------------------------------//
void World::disconnect(int c_id)
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

int World::get_new_client_id()
{
	for (int i = 0; i < MAX_USER; ++i) {
		if (clients[i]._state == ST_FREE) {
			clients[i]._state = ST_ACCEPTED;
			return i;
		}
	}
	return -1;
}

void World::initialize_tilemap()
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

void World::initialize_npc()
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


// ------------------------- //

void World::process_work(OVER_EXP* ex_over, DWORD& num_bytes, ULONG_PTR& key)
{

}

void World::accept_client(OVER_EXP* ex_over, DWORD& num_bytes, ULONG_PTR& key)
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

void World::recv_client(OVER_EXP* ex_over, DWORD& num_bytes, ULONG_PTR& key)
{
	if (0 == num_bytes) disconnect(key);
	int remain_data = num_bytes + clients[key]._prev_remain;

	clients[key].rebuild_packet(ex_over->_send_buf, remain_data);
	clients[key].do_recv();
}

void World::send_client(OVER_EXP* ex_over, DWORD& num_bytes, ULONG_PTR& key)
{
	if (0 == num_bytes) disconnect(key);
	delete ex_over;
}

