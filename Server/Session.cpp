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
	_prevRemainBuffer = 0;
	_sector_x = x / W_SECTOR;
	_sector_y = y / H_SECTOR;
	next_move_time = std::chrono::system_clock::now() + std::chrono::seconds(1);
	_status.calculateMaxExp(*this);
}

void CSession::init(SOCKET& socket, int id)
{
	x = 0;
	y = 0;
	_id = id;
	_name[0] = 0;
	_prevRemainBuffer = 0;
	_socket = socket;
}

// ** view list ** //
void CSession::updateMoveViewList(int client_time, std::unordered_set<int>& new_nl)
{
	sendMovePacket(_id, client_time);

	for (auto n : new_nl)
	{
		if (clients[n]._id == _id) continue;
		if (clients[n]._state == ST_SLEEP)
		{
			clients[n]._state = ST_INGAME;
			std::pair<int, int> id{ n, n };
			World::instance().addEvent(id, EV_MOVE, 1000);
		}
		if (ST_INGAME != clients[n]._state) continue;

		vl.lock();
		if (view_list.count(n) == 0)
		{
			view_list.insert(n);
			vl.unlock();
			
			sendAddObject(n);

			clients[n].checkViewList(_id, client_time);
		}
		else
		{
			vl.unlock();
			clients[n].checkViewList(_id, client_time);
		}

		if (n < MAX_USER) continue;

		clients[n].checkArgoStart(_id);
	}
}

void CSession::checkEraseViewList(std::unordered_set<int>& new_nl)
{
	vl.lock();
	std::unordered_set<int> new_list = view_list;
	vl.unlock();

	for (auto view : new_list)
	{
		if (new_nl.count(view) == 0)
		{
			vl.lock();
			view_list.erase(view);
			vl.unlock();
			removeViewList(view);

			clients[view].vl.lock();
			if (clients[view].view_list.count(_id))
			{
				clients[view].view_list.erase(_id);
				clients[view].vl.unlock();
				clients[view].removeViewList(_id);
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

void CSession::checkViewList(int& c_id, int client_time)
{
	vl.lock();
	if (view_list.count(c_id))
	{
		sendMovePacket(c_id, client_time);
		vl.unlock();
	}
	else
	{
		view_list.insert(c_id);
		vl.unlock();

		sendAddObject(c_id);
	}
}

void CSession::removeViewList(int& view)
{
	if (_id == view) return;
	if (_id >= MAX_USER) return;
	if (_state != ST_INGAME)return;

	sendRemoveObject(view);
}

void CSession::checkInsertViewList(int insert_id)
{
	if (RANGE >= distance(insert_id, _id)) {
		vl.lock();
		view_list.insert(insert_id);
		vl.unlock();
		sendAddObject(insert_id);
	}
}

std::unordered_set<int> CSession::MakeNearList()
{
	int nearDirectionX[9] = { -1,-1,-1,0,0,0,1,1,1 };
	int nearDirectionY[9] = { -1,0,1,-1,0,1,-1,0,1 };
	int h = W_HEIGHT / W_SECTOR;
	int w = W_WIDTH / H_SECTOR;

	std::unordered_set<int> new_near_list;

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

void CSession::processPacket(char* packet)
{
	if (_id < 0 || _id > MAX_USER - 1) return;

	switch (packet[1]) {
		case CS_LOGIN: {
			CS_LOGIN_PACKET* p = reinterpret_cast<CS_LOGIN_PACKET*>(packet);

			auto iter = find_if(g_db_users.begin(), g_db_users.end(), [&p](const USER_DATA user) {
				return strcmp(p->name, user.name) == 0;
			});

			// ** login success ** // 
			if (_state == ST_FREE) {
				break;
			}
			if (_state == ST_INGAME) {
				World::instance().disconnect(_id);
				break;
			}


			if (iter == g_db_users.end())
			{
				do {
					x = rand() % 2000;
					y = rand() % 2000;
				} while (tiles[x][y]);
				
				strcpy_s(_name, p->name);
				_level = 1;
				_exp = 0;
				_hp = 100;

				insertToDatabase(p->name, x, y, _level, _exp, _hp);
			}
			else
			{
				strcpy_s(_name, iter->name);
				x = iter->x;
				y = iter->y;
				_level = iter->level;
				_exp = iter->exp;
				_hp = iter->hp;
			}

			_sendPacket.sendLoginInfoPacket(*this);
			_state = ST_INGAME;

			SetSector(_id);
			CheckMoveSector(_id);

			std::unordered_set<int> new_nl;
			new_nl = MakeNearList();

			for (auto n : new_nl)
			{
				if (clients[n]._id == _id) continue;
				if (clients[n]._state == ST_SLEEP)
				{
					clients[n]._state = ST_INGAME;

					std::pair<int, int>id{ n, n };
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
			processAttack(packet);

			CS_ATTACK_PACKET* p = reinterpret_cast<CS_ATTACK_PACKET*>(packet);
			std::unordered_set<int> new_nl;
			new_nl = MakeNearList();

			for (auto& n : new_nl)
			{
				clients[n].sendAttackPacket(_id,p->skill_type);

				if (p->skill_type == 2) {
					std::pair<int, int> id{ n,_id };
					World::instance().addEvent(id, EV_SKILL_RELEASE, 1500);
				}
			}

			if (p->skill_type == 2) {
				std::pair<int, int> id{ _id,_id };
				World::instance().addEvent(id, EV_SKILL_RELEASE, 1500);
			}

			break;
		}

		case CS_CHAT: {
			CS_CHAT_PACKET* p = reinterpret_cast<CS_CHAT_PACKET*>(packet);
			std::string mess(p->mess);
			chatMessage(mess,1);

			vl.lock();
			std::unordered_set<int> search_vl = view_list;
			vl.unlock();

			for (int p_id : search_vl)
			{
				if (p_id >= MAX_USER) continue;
				clients[p_id].chatMessage(mess, 1);
			}
		}
	}
}

void CSession::moveObject(char* packet)
{
	CS_MOVE_PACKET* p = reinterpret_cast<CS_MOVE_PACKET*>(packet);

	updateMoveClients(_id, p->direction);
	CheckMoveSector(_id);

	std::unordered_set<int> new_nl;
	new_nl = MakeNearList();

	updateMoveViewList(p->client_time, new_nl);
	checkEraseViewList(new_nl);
}

void CSession::processAttack(char* packet)
{
	CS_ATTACK_PACKET* p = reinterpret_cast<CS_ATTACK_PACKET*>(packet);
	vl.lock();
	std::unordered_set<int> search_vl = view_list;
	vl.unlock();

	setSkillCoolDown(p);
	
	if (p->skill_type == 2) {
		_isShield = true;
		std::pair<int, int> id{ _id,p->skill_type };
		World::instance().addEvent(id, EV_SKILL_COOL, 3000);
		return;
	}

	for (int mon : search_vl)
	{
		if (p->skill_type == 0) {
			if (isMonsterCollisionNormalAttack(mon, _id) == false)
				continue;
		}
		else if (p->skill_type == 1) {
			if (isMonsterCollisionAttack(mon, _id) == false)
				continue;
		}

		int damage = _level * 2 * (p->skill_type + 1);

		bool is_dying = clients[mon].decreaseHp(damage);
		clients[mon].setPeaceTarget(_id);
		
		std::string mess = "User:" + std::to_string(_id) + " attack " + clients[mon]._name
			+ "and "+ std::to_string(damage) + " Damage";
		chatMessage(mess);

		if (is_dying)
		{
			_status.updateExp(*this, mon);
			sendChangeStatusPacket(_id);
			clients[mon].readyToRespawn();
		}
	}
}

void CSession::setSkillCoolDown(CS_ATTACK_PACKET* p)
{
	switch (p->skill_type)
	{
		case 0: {
			std::pair<int, int> id{ _id,p->skill_type };
			World::instance().addEvent(id, EV_SKILL_COOL, 400);
			break;
		}
		case 1: {
			std::pair<int, int> id{ _id,p->skill_type };
			World::instance().addEvent(id, EV_SKILL_COOL, 1500);
			break;
		}
		case 3:
		{
			std::pair<int, int> id{ _id,p->skill_type };
			World::instance().addEvent(id, EV_SKILL_COOL, 5000);
			break;
		}
	}
}

void CSession::chatMessage(std::string& mess, int id)
{
	sendChatPacket(id, mess.c_str());
}

void CSession::rebuildPacket(char* send_buffer, int& remain)
{
	char* temp = send_buffer;
	
	while (remain > 0) 
	{
		REBUILD_PACKET* packet = reinterpret_cast<REBUILD_PACKET*>(temp);
		if (packet->size > remain) break;

		processPacket(temp);
		temp += packet->size;
		remain -= packet->size;
	}

	_prevRemainBuffer = remain;
	if (_prevRemainBuffer > 0) memcpy(send_buffer, temp, _prevRemainBuffer);
}

// **              ** //

void CSession::doRecv()
{
	DWORD recv_flag = 0;
	memset(&_recv_over._over, 0, sizeof(_recv_over._over));
	_recv_over._wsabuf.len = BUF_SIZE - _prevRemainBuffer;
	_recv_over._wsabuf.buf = _recv_over._send_buf + _prevRemainBuffer;
	WSARecv(_socket, &_recv_over._wsabuf, 1, 0, &recv_flag, &_recv_over._over, 0);
}

void CSession::doSend(void* packet)
{
	OVER_EXP* sdata = new OVER_EXP{ reinterpret_cast<char*>(packet) };
	WSASend(_socket, &sdata->_wsabuf, 1, 0, 0, &sdata->_over, 0);
}


// ** packet send ** //

void CSession::sendMovePacket(int c_id, int client_time)
{
	_sendPacket.sendMovePacket(*this, c_id, client_time);
}

void CSession::sendAddObject(int c_id)
{
	_sendPacket.sendAddObject(*this, c_id);
}

void CSession::sendChatPacket(int c_id, const char* mess)
{
	_sendPacket.sendChatPacket(*this, c_id,mess);
}

void CSession::sendChangeStatusPacket(int c_id)
{
	_sendPacket.sendChangeStatusPacket(*this, c_id);
}

void CSession::sendAttackPacket(int c_id, int skill_type, short active_type, short x , short y)
{
	_sendPacket.sendAttackPacket(*this, c_id, skill_type , active_type, x , y);
}

void CSession::sendMonsterAttack(int id, std::string& mess)
{
	std::unordered_set<int> new_nl;
	new_nl = MakeNearList();

	sendAttackPacket(id, 0 ,0, x, y);
	chatMessage(mess);
	for (auto p_id : new_nl)
	{
		if (p_id >= MAX_USER) continue;
		clients[p_id].sendAttackPacket(id, 0 , 0, x , y);
		clients[p_id].chatMessage(mess);
	}
}

void CSession::sendRemoveObject(int c_id)
{
	_sendPacket.sendRemoveObject(*this,c_id);
}

// ** status ** // 

bool CSession::decreaseHp(int hp)
{
	bool isDying = _status.decreaseHp(*this, hp);
	checkHealing();

	return isDying;
}

void CSession::heal()
{
	bool isHeal = _status.healHp(*this);

	if (isHeal)
	{
		_sendPacket.sendChangeStatusPacket(*this, _id);
		std::string mess = "player Heal!!";
		chatMessage(mess);
	}
}

void CSession::checkHealing()
{
	if (_isHealing) return;
	_isHealing = true;

	std::pair<int, int> id{ _id,_id };
	World::instance().addEvent(id, EV_HEAL, 5000);
}


// ** Respawn ** //

void CSession::respawnPlayer()
{
	_status.respawn(*this);

	std::string name(_name);
	auto iter = find_if(g_db_users.begin(), g_db_users.end(), [&name](const USER_DATA user) {
		return strcmp(name.c_str(), user.name) == 0;
	});

	x = iter->x; y = iter->y;

	CheckMoveSector(_id);
	std::unordered_set<int> new_nl;
	new_nl = MakeNearList();

	updateMoveViewList(0, new_nl);
	checkEraseViewList(new_nl);
}

void CSession::setRespawnStatus()
{
	_status.respawn(*this);
}

void CSession::readyToRespawn()
{
	_state = ST_FREE;
	
	std::unordered_set<int> new_nl;
	new_nl = MakeNearList();

	for (auto p_id : new_nl) 
	{
		if (p_id >= MAX_USER) continue;

		clients[p_id].vl.lock();
		clients[p_id].view_list.erase(_id);
		clients[p_id].vl.unlock();
		clients[p_id].removeViewList(_id);
	}

	vl.lock();
	view_list.clear();
	vl.unlock();

	std::pair<int, int> id{ _id,_id };
	World::instance().addEvent(id, EV_RESPAWN, 300000);
}

void CSession::releaseShield()
{
	_isShield = false;
}

bool CSession::getShield()
{
	return _isShield;
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
		updateMoveClients(_id, dir);
	}
	else if (_target_id != -1)
	{
		movePathToNpc();
	}

	CheckMoveSector(_id);
}

void CSession::movePathToNpc()
{
	_pathl.lock();
	int target = _target_id;
	_pathl.unlock();

	if (clients[target]._isShield) return;

	_pathl.lock();
	bool isFind = _astar.searchRoad(clients[_target_id].x, clients[_target_id].y, x, y);
	_pathl.unlock();

	if (isFind == false) {
		_target_id = -1;
		return;
	}

	_pathl.lock();
	bool isAttack = _astar.getPathPosition(&x,&y);
	_pathl.unlock();


	if (isAttack == false) return;
	if (_isAttack == false) return;


	_pathl.lock();
	_isAttack = false;
	bool isDying = clients[_target_id].decreaseHp(_level);
	if (isDying) {
		clients[_target_id].respawnPlayer();
	}

	clients[_target_id].sendChangeStatusPacket(_target_id);
	std::string mess = "Monster:" + std::to_string(_id) + " attack "
		+ clients[_target_id]._name + " and " + std::to_string(_level) + " Damage";
	clients[_target_id].sendMonsterAttack(_id, mess);

	std::pair<int, int> id{ _id,_target_id };
	_pathl.unlock();

	World::instance().addEvent(id, EV_ATTACK_ACTIVE, 1000);
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

//