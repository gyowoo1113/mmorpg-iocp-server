#include "pch.h"
#include "Session.h"


// ** init ** //


CSession::CSession()
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
	_status.calculateMaxExp(*this);
}

void CSession::init(SOCKET& socket, int id)
{
	x = 0;
	y = 0;
	_id = id;
	_name[0] = 0;
	_prev_remain = 0;
	_socket = socket;
}

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
				_sendPacket.send_login_fail(*this);
				return;
			}

			// ** login success ** // 
			if (_state == ST_FREE) {
				break;
			}
			if (_state == ST_INGAME) {
				World::instance().disconnect(_id);
				break;
			}


			strcpy_s(_name, iter->name);
			x = iter->x;
			y = iter->y;
			_level = iter->level;
			_exp = iter->exp;
			_hp = iter->hp;
			_sendPacket.send_login_info_packet(*this);
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
			moveObject(packet);

			break;
		}

		case CS_ATTACK: {
			process_attack(packet);

			break;
		}
	}
}

void CSession::moveObject(char* packet)
{
	CS_MOVE_PACKET* p = reinterpret_cast<CS_MOVE_PACKET*>(packet);

	update_move_clients(_id, p->direction);
	CheckMoveSector(_id);

	unordered_set<int> new_nl;
	new_nl = MakeNearList();

	update_move_view_list(p, new_nl);
	check_erase_view_list(new_nl);
}

void CSession::process_attack(char* packet)
{
	CS_ATTACK_PACKET* p = reinterpret_cast<CS_ATTACK_PACKET*>(packet);
	vl.lock();
	unordered_set<int> search_vl =view_list;
	vl.unlock();

	for (int mon : search_vl)
	{
		if (isMonsterCollisionAttack(mon, _id) == false)
			continue;

		bool is_dying = clients[mon].decreaseHp(50);
		
		string mess = "User:" + to_string(_id) + " attack " + clients[mon]._name + ", 50 Damage";
		chatSystemMessage(mess);

		if (is_dying)
		{
			_status.updateExp(*this, mon);
			send_change_status_packet(_id);
		}
	}
}

void CSession::chatSystemMessage(std::string& mess)
{
	send_chat_packet(-1, mess.c_str());
}

void CSession::rebuild_packet(char* send_buffer, int& remain)
{
	char* temp = send_buffer;
	
	while (remain > 0) 
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

void CSession::send_move_packet(int c_id, int client_time)
{
	_sendPacket.send_move_packet(*this, c_id, client_time);
}

void CSession::send_add_object(int c_id)
{
	_sendPacket.send_add_object(*this, c_id);
}

void CSession::send_chat_packet(int c_id, const char* mess)
{
	_sendPacket.send_chat_packet(*this, c_id,mess);
}

void CSession::send_change_status_packet(int c_id)
{
	_sendPacket.send_change_status_packet(*this, c_id);
}

void CSession::send_remove_object(int c_id)
{
	_sendPacket.send_remove_object(*this,c_id);
}

// ** status ** // 

bool CSession::decreaseHp(int hp)
{
	return _status.decreaseHp(*this,hp);
}

void CSession::heal()
{
	_status.healHp(*this);
}

// ** Monster ** // 

void CSession::setMonsterTypes()
{
	monsterType = rand() % 2;
	monsterMoveType = rand() % 2;
}

bool CSession::isMonsterMoving(){
	return (_target_id != -1 || monsterMoveType);
}