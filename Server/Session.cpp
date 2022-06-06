#include "pch.h"
#include "Session.h"

// ** view list ** //
void CSession::update_move_view_list(CS_MOVE_PACKET* p, std::unordered_set<int>& new_nl)
{
	send_move_packet(_id, p->client_time);

	for (auto n : new_nl)
	{
		if (clients[n]._id == _id) continue;
		if (ST_INGAME != clients[n]._state) continue;

		vl.lock();
		if (view_list.count(n) == 0)
		{
			view_list.insert(n);
			vl.unlock();
			
			send_add_object(n);

			clients[n].check_view_list(_id, p);
		}
		else
		{
			vl.unlock();
			clients[n].check_view_list(_id, p);
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
			remove_view_list(view);

			clients[view].vl.lock();
			if (clients[view].view_list.count(_id))
			{
				clients[view].view_list.erase(_id);
				clients[view].vl.unlock();
				clients[view].remove_view_list(_id);
			}
			else
			{
				clients[view].vl.unlock();
			}
		}
	}
}

void CSession::check_view_list(int& c_id, CS_MOVE_PACKET* p)
{
	vl.lock();
	if (view_list.count(c_id))
	{
		send_move_packet(c_id, p->client_time);
		vl.unlock();
	}
	else
	{
		view_list.insert(c_id);
		vl.unlock();

		send_add_object(c_id);
	}
}

void CSession::remove_view_list(int& view)
{
	if (_id == view) return;
	if (_id >= MAX_USER) return;
	if (_state != ST_INGAME)return;

	send_remove_object(view);
}

unordered_set<int> CSession::MakeNearList()
{
	int nearDirectionX[9] = { -1,-1,-1,0,0,0,1,1,1 };
	int nearDirectionY[9] = { -1,0,1,-1,0,1,-1,0,1 };
	int h = W_HEIGHT / 10;
	int w = W_WIDTH / 10;

	unordered_set<int> new_near_list;

	for (int i = 0; i < 9; ++i)
	{
		int dirX = _sector_x + nearDirectionX[i];
		int dirY = _sector_y + nearDirectionY[i];

		if (dirX < 0 || dirY < 0 || dirX > w - 1 || dirY > h - 1) continue;

		for (auto id : sector[dirX][dirY])
		{
			if (_id == id) continue;
			if (RANGE >= distance(_id, id))
			{
				new_near_list.insert(id);
			}
		}
	}

	return (new_near_list);
}

// ** packet process ** //

void CSession::process_packet(char* packet)
{
	if (_id < 0 || _id > MAX_USER - 1) return;

	switch (packet[1]) {
		case CS_LOGIN: {
			CS_LOGIN_PACKET* p = reinterpret_cast<CS_LOGIN_PACKET*>(packet);

			auto iter = find_if(g_db_users.begin(), g_db_users.end(), [&p](const USER_DATA user) {
				return strcmp(p->name, user.name) == 0;
			});

			if (iter == g_db_users.end())
			{
				send_login_fail();
				return;
			}

			// ** login success ** // 
			if (_state == ST_FREE) {
				break;
			}
			if (_state == ST_INGAME) {
				disconnect(_id);
				break;
			}

			strcpy_s(_name, iter->name);
			x = iter->x;
			y = iter->y;
			_level = iter->level;
			_exp = iter->exp;
			_hp = iter->hp;
			send_login_info_packet();
			_state = ST_INGAME;

			SetSector(_id);

			for (int i = 0; i < MAX_USER; ++i) {
				auto& pl = clients[i];
				if (pl._id == _id) continue;
				if (ST_INGAME != pl._state) {
					continue;
				}
				if (RANGE >= distance(_id, pl._id)) {
					pl.vl.lock();
					pl.view_list.insert(_id);
					pl.vl.unlock();
					pl.send_add_object(_id);
				}
			}

			for (auto& pl : clients) {
				if (pl._id == _id) continue;
				if (ST_INGAME != pl._state) continue;

				if (RANGE >= distance(pl._id, _id)) {
					vl.lock();
					view_list.insert(pl._id);
					vl.unlock();
					send_add_object(pl._id);
				}
			}


			break;
		}

		case CS_MOVE: {
			CS_MOVE_PACKET* p = reinterpret_cast<CS_MOVE_PACKET*>(packet);

			update_move_clients(_id, p->direction);
			CheckMoveSector(_id);

			unordered_set<int> new_nl;
			new_nl = MakeNearList();

			update_move_view_list(p, new_nl);
			check_erase_view_list(new_nl);

			break;
		}

		case CS_ATTACK: {
			CS_ATTACK_PACKET* p = reinterpret_cast<CS_ATTACK_PACKET*>(packet);
			process_attack();

			break;
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

		bool is_dying = clients[mon].decreaseHp(50);
		
		if (is_dying)
		{
			_status.updateExp(*this);
			send_change_status_packet(_id);
		}

		for (int i = 0; i < MAX_USER; ++i)
		{
			if (ST_INGAME != clients[i]._state) continue;
			
			string mess = "User:" + to_string(_id) + " attack to " + clients[mon]._name;
			clients[i].send_chat_packet(-1, mess.c_str());
		}

	}
}

void CSession::rebuild_packet(char* send_buffer, int& remain)
{
	char* temp = send_buffer;
	
	while (remain != 0) 
	{
		REBUILD_PACKET* packet = reinterpret_cast<REBUILD_PACKET*>(temp);
		if (packet->size > remain) break;

		process_packet(temp);
		temp += packet->size;
		remain -= packet->size;
	}

	_prev_remain = remain;
	if (_prev_remain > 0) memcpy(send_buffer, temp, _prev_remain);
}

// ** packet send ** //

void CSession::send_login_fail()
{
	SC_LOGIN_FAIL_PACKET p;
	p.size = sizeof(SC_LOGIN_FAIL_PACKET);
	p.type = SC_LOGIN_FAIL;
	do_send(&p);
}

void CSession::send_login_info_packet()
{
	SC_LOGIN_INFO_PACKET p;
	p.id = _id;
	p.size = sizeof(SC_LOGIN_INFO_PACKET);
	p.type = SC_LOGIN_INFO;
	p.x = x;
	p.y = y;
	p.hp = _hp;
	p.exp = _exp;
	p.level = _level;
	do_send(&p);
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

void CSession::send_change_status_packet(int c_id)
{
	SC_CHANGE_STATUS_PACKET p;
	p.size = sizeof(SC_CHANGE_STATUS_PACKET);
	p.type = SC_CHANGE_STATUS;
	p.id = c_id;
	p.hp = _hp;
	p.exp = _exp;
	p.level = _level;
	do_send(&p);
}

void CSession::send_remove_object(int c_id)
{
	SC_REMOVE_PLAYER_PACKET p;
	p.id = c_id;
	p.size = sizeof(SC_REMOVE_PLAYER_PACKET);
	p.type = SC_REMOVE_PLAYER;
	do_send(&p);
}

// ** status ** // 

bool CSession::decreaseHp(int hp)
{
	return _status.decreaseHp(*this,hp);
}