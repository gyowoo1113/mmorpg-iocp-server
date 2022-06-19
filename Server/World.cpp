#pragma once
#include "pch.h"
#include "World.h"

World World::_instance;

World& World::instance()
{
	return _instance;
}

// ------------ EventTimer -------------- //

CEventTimer& World::getTimer()
{
	if (_timer == nullptr){
		_timer = new CEventTimer();
	}
	return *_timer;
}

void World::process_event(TIMER_EVENT& avent)
{
	auto ex_over = new OVER_EXP;
	ex_over->_comp_type = avent.ev;
	ex_over->target_id = avent.id.second;
	PostQueuedCompletionStatus(handle_iocp, 1, avent.id.first, &ex_over->_over);
}

void World::addEvent(std::pair<int, int> id, COMP_TYPE type, int time)
{
	std::lock_guard<std::mutex> tt{ _lock };
	TIMER_EVENT avent{ id,type,std::chrono::system_clock::now() + std::chrono::milliseconds(time) };
	getTimer().pushEvent(avent);
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
		pl.sendRemoveObject(c_id);
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
	std::ifstream in("../Resource/objects.txt");
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

	CAstar astar;
	astar.initMapClosedList();
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
		clients[npc_id]._state = ST_SLEEP;
		clients[npc_id].setMonsterTypes();
		clients[npc_id]._level = rand() % 10 + 1;
		clients[npc_id]._maxHp = clients[npc_id]._hp = 30;
		SetSector(i);
		sprintf_s(clients[npc_id]._name, "M-%d", npc_id);
	}
}


// ----- accept, recv, send ------- //

void World::accept_client(OVER_EXP* ex_over, DWORD& num_bytes, ULONG_PTR& key)
{
	SOCKET c_socket = reinterpret_cast<SOCKET>(ex_over->_wsabuf.buf);
	int client_id = get_new_client_id();
	if (client_id != -1) {
		clients[client_id].init(c_socket,client_id);
		CreateIoCompletionPort(reinterpret_cast<HANDLE>(c_socket), handle_iocp, client_id, 0);
		clients[client_id].doRecv();
		c_socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	}
	else {
		std::cout << "Max user exceeded.\n";
	}
	ZeroMemory(&ex_over->_over, sizeof(ex_over->_over));
	ex_over->_wsabuf.buf = reinterpret_cast<CHAR*>(c_socket);
	int addr_size = sizeof(SOCKADDR_IN);
	AcceptEx(server_socket, c_socket, ex_over->_send_buf, 0, addr_size + 16, addr_size + 16, 0, &ex_over->_over);
}

void World::recv_client(OVER_EXP* ex_over, DWORD& num_bytes, ULONG_PTR& key)
{
	if (0 == num_bytes) disconnect(key);
	int remain_data = num_bytes + clients[key]._prevRemainBuffer;

	clients[key].rebuildPacket(ex_over->_send_buf, remain_data);
	clients[key].doRecv();
}

void World::send_client(OVER_EXP* ex_over, DWORD& num_bytes, ULONG_PTR& key)
{
	if (0 == num_bytes) disconnect(key);
	delete ex_over;
}

// ------ Event ------------------ // 

void World::moveNpcEvent(OVER_EXP* ex_over, DWORD& num_bytes, ULONG_PTR& key)
{
	move_npc(key);
}

void World::healEvent(OVER_EXP* ex_over, DWORD& num_bytes, ULONG_PTR& key)
{
	clients[key].heal();
}

void World::monsterAttackEvent(OVER_EXP* ex_over, DWORD& num_bytes, ULONG_PTR& key)
{

}

void World::AttackActiveEvent(OVER_EXP* ex_over, DWORD& num_bytes, ULONG_PTR& key)
{
	clients[key].setAttack(true);
}

void World::npcRespawnEvent(OVER_EXP* ex_over, DWORD& num_bytes, ULONG_PTR& key)
{
	clients[key]._state = ST_SLEEP;
	clients[key].setRespawnStatus();

	std::unordered_set<int> new_nl;
	new_nl = clients[key].MakeNearList();

	for (auto p_id : new_nl) {
		if (p_id >= MAX_USER) continue;

		clients[p_id].vl.lock();
		if (clients[p_id].view_list.count(key) == 0) {
			clients[p_id].view_list.insert(key);
			clients[p_id].vl.unlock();
			clients[p_id].sendAddObject(key);
		}
		else {
			clients[p_id].sendMovePacket(key, 0);
			clients[p_id].vl.unlock();
		}
		clients[key].checkArgoStart(p_id);
	}
}
