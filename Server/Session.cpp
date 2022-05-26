#include "pch.h"
#include "Session.h"

void CSession::send_remove_object(int c_id)
{
	SC_REMOVE_PLAYER_PACKET p;
	p.id = c_id;
	p.size = sizeof(SC_REMOVE_PLAYER_PACKET);
	p.type = SC_REMOVE_PLAYER;
	do_send(&p);
}

void CSession::update_move_view_list(CS_MOVE_PACKET* p, std::unordered_set<int>& new_nl)
{
	send_move_packet(_id, p->client_time);

	for (auto n : new_nl)
	{
		if (clients[n]._id == _id) continue;
		lock_guard<mutex> aa{ clients[n]._sl };
		if (ST_INGAME != clients[n]._s_state) continue;

		vl.lock();
		if (view_list.count(n) == 0)
		{
			view_list.insert(n);
			vl.unlock();
			
			send_add_object(n);

			check_view_list(n, _id, p);

		}
		else
		{
			vl.unlock();
			check_view_list(n, _id, p);
		}
	}
}

void CSession::check_erase_view_list(std::unordered_set<int>& new_nl)
{
	vl.lock();
	unordered_set<int> new_list = view_list;
	vl.unlock();

	for (auto view : new_list)
	{
		if (new_nl.count(view) == 0)
		{
			vl.lock();
			view_list.erase(view);
			vl.unlock();
			remove_view_list(_id, view);

			clients[view].vl.lock();
			if (clients[view].view_list.count(_id))
			{
				clients[view].view_list.erase(_id);
				clients[view].vl.unlock();
				remove_view_list(view, _id);
			}
			else
			{
				clients[view].vl.unlock();
			}
		}
	}
}

void CSession::process_attack()
{
	vl.lock();
	unordered_set<int> search_vl =view_list;
	vl.unlock();

	for (int mon : search_vl)
	{
		if (isMonsterCollisionAttack(mon, _id) == false)
			continue;

		for (int i = 0; i < MAX_USER; ++i)
		{
			lock_guard<mutex> aa{ clients[i]._sl };
			if (ST_INGAME != clients[i]._s_state) continue;
			
			string mess = "User:" + to_string(_id) + " attack to " + clients[mon]._name;
			clients[i].send_chat_packet(-1, mess.c_str());
		}

	}
}

void CSession::send_move_packet(int c_id, int client_time)
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

void CSession::send_add_object(int c_id)
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

void CSession::send_chat_packet(int c_id, const char* mess)
{
	SC_CHAT_PACKET p;
	p.id = c_id;
	p.size = sizeof(SC_CHAT_PACKET) - sizeof(p.mess) + strlen(mess) + 1;
	p.type = SC_CHAT;
	strcpy_s(p.mess, mess);
	do_send(&p);
}
