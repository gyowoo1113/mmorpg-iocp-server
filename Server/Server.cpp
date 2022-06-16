#include "../Client/protocol.h"
#include "pch.h"
#include "Session.h"
#include "Astar.h"

using namespace std;
using namespace sf;
#pragma comment (lib, "WS2_32.LIB")
#pragma comment(lib, "MSWSock.lib")

const short SERVER_PORT = 4000;
const int BUFSIZE = 256;


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
			if (tiles[x - 1][y] == 0)
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


void move_npc(int npc_id)
{
	if (clients[npc_id]._state == ST_SLEEP) return;
	if (clients[npc_id].isMonsterMoving() == false)return;

	unordered_set<int> old_vl;
	for (int i = 0; i < MAX_USER; ++i)
	{
		if (clients[i]._state != ST_INGAME) continue;
		if (distance(npc_id, i) <= RANGE) old_vl.insert(i);
	}

	clients[npc_id].moveMonster();


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
		}
		else {
			clients[p_id].send_move_packet(npc_id, 0);
			clients[p_id].vl.unlock();
		}
		clients[npc_id].checkArgoStart(p_id);
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

	if (new_vl.empty())
	{
		clients[npc_id]._state = ST_SLEEP;
		return;
	}

	pair<int, int> id{ npc_id, npc_id};
	World::instance().addEvent(id, COMP_TYPE::EV_MOVE, 1000);
}


typedef void (World::* member_funcion_pointer)(OVER_EXP*, DWORD&, ULONG_PTR&);

void do_worker()
{
	while (true) {
		HANDLE& handle_iocp = World::instance().getHandle();

		DWORD num_bytes;
		ULONG_PTR key;
		WSAOVERLAPPED* over = nullptr;
		BOOL ret = GetQueuedCompletionStatus(handle_iocp, &num_bytes, &key, &over, INFINITE);
		OVER_EXP* ex_over = reinterpret_cast<OVER_EXP*>(over);
		if (FALSE == ret) {
			if (ex_over->_comp_type == OP_ACCEPT) cout << "Accept Error";
			else {
				cout << "GQCS Error on client[" << key << "]\n";
				World::instance().disconnect(static_cast<int>(key));
				if (ex_over->_comp_type == OP_SEND) delete ex_over;
				continue;
			}
		}

		member_funcion_pointer fp[static_cast<int>(COMP_TYPE::length)]
			= { &World::accept_client, &World::recv_client,&World::send_client,
		&World::moveNpcEvent,&World::healEvent,&World::monsterAttackEvent};

		(World::instance().*fp[static_cast<int>(ex_over->_comp_type)])(ex_over, num_bytes, key);
	}
}


void eventTimer()
{
	while (true)
	{
		this_thread::sleep_for(10ms);
		World::instance().getTimer().update();
	}
}

int main()
{
	World::instance().initialize_tilemap();
	World::instance().initialize_npc();
	load_database();

	WSADATA WSAData;
	WSAStartup(MAKEWORD(2, 2), &WSAData);

	SOCKET& server_socket = World::instance().getServerSocket();
	HANDLE& handle_iocp = World::instance().getHandle();
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
	for (int i = 0; i < 5; ++i)
		worker_threads.emplace_back(do_worker);
	thread timer_thread{ eventTimer };

	timer_thread.join();
	for (auto& th : worker_threads)
		th.join();

	closesocket(server_socket);
	WSACleanup();
}