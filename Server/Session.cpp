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
void CSession::update_move_view_list(int client_time, std::unordered_set<int>& new_nl)
{
	send_move_packet(_id, client_time);

	for (auto n : new_nl)
	{
		if (clients[n]._id == _id) continue;
		if (clients[n]._state == ST_SLEEP)
		{
			clients[n]._state = ST_INGAME;
			pair<int, int> id{ n, n };
			World::instance().addEvent(id, EV_MOVE, 1000);
		}
		if (ST_INGAME != clients[n]._state) continue;

		vl.lock();
		if (view_list.count(n) == 0)
		{
			view_list.insert(n);
			vl.unlock();
			
			send_add_object(n);

			clients[n].check_view_list(_id, client_time);
		}
		else
		{
			vl.unlock();
			clients[n].check_view_list(_id, client_time);
		}

		if (n < MAX_USER) continue;

		clients[n].checkArgoStart(_id);
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

			if (view < MAX_USER) continue;
			clients[view].checkTargetRelease(_id);
		}
	}
}

void CSession::check_view_list(int& c_id, int client_time)
{
	vl.lock();
	if (view_list.count(c_id))
	{
		send_move_packet(c_id, client_time);
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

void CSession::checkInsertViewList(int insert_id)
{
	if (RANGE >= distance(insert_id, _id)) {
		vl.lock();
		view_list.insert(insert_id);
		vl.unlock();
		send_add_object(insert_id);
	}
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
			if (clients[id]._state == ST_FREE) continue;
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
			CheckMoveSector(_id);

			unordered_set<int> new_nl;
			new_nl = MakeNearList();

			for (auto n : new_nl)
			{
				if (clients[n]._id == _id) continue;
				if (clients[n]._state == ST_SLEEP)
				{
					clients[n]._state = ST_INGAME;

					pair<int, int>id{ n, n };
					World::instance().addEvent(id, EV_MOVE, 1000);
				}
				if (ST_INGAME != clients[n]._state) continue;

				checkInsertViewList(n);

				if (n < MAX_USER)
					clients[n].checkInsertViewList(_id);
			}

			break;
		}

		case CS_MOVE: {
			moveObject(packet);

			break;
		}

		case CS_ATTACK: {
			process_attack(packet);
			unordered_set<int> new_nl;
			new_nl = MakeNearList();

			for (auto& n : new_nl)
			{
				clients[n].send_attack_packet(_id, 0);
			}
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

	update_move_view_list(p->client_time, new_nl);
	check_erase_view_list(new_nl);
}

void CSession::process_attack(char* packet)
{
	CS_ATTACK_PACKET* p = reinterpret_cast<CS_ATTACK_PACKET*>(packet);
	vl.lock();
	unordered_set<int> search_vl = view_list;
	vl.unlock();

	for (int mon : search_vl)
	{
		if (isMonsterCollisionAttack(mon, _id) == false)
			continue;

		bool is_dying = clients[mon].decreaseHp(50);
		clients[mon].setPeaceTarget(_id);
		
		string mess = "User:" + to_string(_id) + " attack " + clients[mon]._name + ", 50 Damage";
		chatSystemMessage(mess);

		if (is_dying)
		{
			_status.updateExp(*this, mon);
			send_change_status_packet(_id);
			clients[mon].readyToRespawn();
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

void CSession::send_attack_packet(int c_id, int skill_type , short x , short y)
{
	_sendPacket.send_attack_packet(*this, c_id, skill_type , x , y);
}

void CSession::sendMonsterAttack(int id, string& mess)
{
	unordered_set<int> new_nl;
	new_nl = MakeNearList();

	send_attack_packet(id, 0 , x, y);
	chatSystemMessage(mess);
	for (auto p_id : new_nl)
	{
		clients[p_id].send_attack_packet(id, 0 , x , y);
		clients[p_id].chatSystemMessage(mess);
	}
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

// ** Respawn ** //

void CSession::respawnPlayer()
{
	_status.respawn(*this);

	string name(_name);
	auto iter = find_if(g_db_users.begin(), g_db_users.end(), [&name](const USER_DATA user) {
		return strcmp(name.c_str(), user.name) == 0;
	});

	x = iter->x; y = iter->y;

	CheckMoveSector(_id);
	unordered_set<int> new_nl;
	new_nl = MakeNearList();

	update_move_view_list(0, new_nl);
	check_erase_view_list(new_nl);
}

void CSession::setRespawnStatus()
{
	_status.respawn(*this);
}

void CSession::readyToRespawn()
{
	_state = ST_FREE;
	
	unordered_set<int> new_nl;
	new_nl = MakeNearList();

	for (auto p_id : new_nl) 
	{
		if (p_id >= MAX_USER) continue;

		clients[p_id].vl.lock();
		clients[p_id].view_list.erase(_id);
		clients[p_id].vl.unlock();
		clients[p_id].remove_view_list(_id);
	}

	vl.lock();
	view_list.clear();
	vl.unlock();

	pair<int, int> id{ _id,_id };
	World::instance().addEvent(id, EV_RESPAWN, 300000);
}

// ** Monster ** // 

void CSession::setMonsterTypes()
{
	monsterType = rand() % 2;
	monsterMoveType = rand() % 2;
}

void CSession::moveMonster()
{
	if (_target_id == -1 && monsterMoveType == 1) 
	{
		char dir = static_cast<char>(rand() % 4);
		update_move_clients(_id, dir);
	}
	else if (_target_id != -1)
	{
		movePathToNpc();
	}

	CheckMoveSector(_id);
}

void CSession::movePathToNpc()
{
	CAstar astar;
	pair<int, int> pos;

	lock_guard<mutex> pp{ _pathl };
	astar.searchRoad(pathfind_pos, x, y, clients[_target_id].x, clients[_target_id].y);

	if (pathfind_pos.empty()) return;

	if (pathfind_pos.size() != 1)
	{
		pos = pathfind_pos.top();
		pathfind_pos.pop();

		x = pos.first; y = pos.second;
	}
	else
	{
		if (_isAttack == false) return;

		_isAttack = false;
		bool isDying = clients[_target_id].decreaseHp(_level);
		clients[_target_id].send_change_status_packet(_target_id);

		string mess = "Monster:" + to_string(_id) + " attack " + clients[_target_id]._name + "," + to_string(_level) + " Damage";
		clients[_target_id].sendMonsterAttack(_id, mess);

		pair<int, int> id{ _id,_target_id };
		World::instance().addEvent(id, EV_ATTACK_ACTIVE, 1000);

		if (isDying) {
			clients[_target_id].respawnPlayer();
		}
	}
}

void CSession::setPeaceTarget(int id)
{
	if (monsterType != 0) return;
	if (_target_id != -1) return;

	_target_id = id;
}

void CSession::setArgoTarget(int id)
{
	if (_target_id != -1) return;
	if (monsterType != 1) return;

	_target_id = id;
}

void CSession::checkArgoStart(int c_id)
{
	if (monsterType != 1) return;
	if (_target_id != -1) return;

	if (distance(c_id, _id) <= MONSTER_RANGE)
		setArgoTarget(c_id);
}
