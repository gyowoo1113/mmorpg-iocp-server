#include "pch.h"
#include "SendPacket.h"
#include "Session.h"

void CSendPacket::sendLoginFail(CSession& client)
{
	SC_LOGIN_FAIL_PACKET p;
	p.size = sizeof(SC_LOGIN_FAIL_PACKET);
	p.type = SC_LOGIN_FAIL;
	client.doSend(&p);
}

void CSendPacket::sendLoginInfoPacket(CSession& client)
{
	SC_LOGIN_OK_PACKET p;
	p.id = client._id;
	p.size = sizeof(SC_LOGIN_OK_PACKET);
	p.type = SC_LOGIN_OK;
	p.x = client.x;
	p.y = client.y;
	p.hp = client._hp;
	p.exp = client._exp;
	p.level = client._level;
	client.doSend(&p);
}

void CSendPacket::sendMovePacket(CSession& client, int c_id, int client_time)
{
	SC_MOVE_OBJECT_PACKET p;
	p.id = c_id;
	p.size = sizeof(SC_MOVE_OBJECT_PACKET);
	p.type = SC_MOVE_OBJECT;
	p.x = clients[c_id].x;
	p.y = clients[c_id].y;
	p.client_time = client_time;
	client.doSend(&p);
}

void CSendPacket::sendAddObject(CSession& client, int c_id)
{
	SC_ADD_OBJECT_PACKET p;
	p.id = c_id;
	p.size = sizeof(SC_ADD_OBJECT_PACKET);
	p.type = SC_ADD_OBJECT;
	p.x = clients[c_id].x;
	p.y = clients[c_id].y;
	p.race = clients[c_id].monsterType * 2 +  clients[c_id].monsterMoveType;
	strcpy_s(p.name, clients[c_id]._name);
	client.doSend(&p);
}

void CSendPacket::sendRemoveObject(CSession& client, int c_id)
{
	SC_REMOVE_OBJECT_PACKET p;
	p.id = c_id;
	p.size = sizeof(SC_REMOVE_OBJECT_PACKET);
	p.type = SC_REMOVE_OBJECT;
	client.doSend(&p);
}

void CSendPacket::sendChatPacket(CSession& client, int c_id, const char* mess)
{
	SC_CHAT_PACKET p;
	p.id = c_id;
	p.size = sizeof(SC_CHAT_PACKET) - sizeof(p.mess) + strlen(mess) + 1;
	p.type = SC_CHAT;
	strcpy_s(p.mess, mess);
	client.doSend(&p);
}

void CSendPacket::sendChangeStatusPacket(CSession& client, int c_id)
{
	SC_STAT_CHANGE_PACKET p;
	p.size = sizeof(SC_STAT_CHANGE_PACKET);
	p.type = SC_STAT_CHANGE;
	p.id = c_id;
	p.hp = clients[c_id]._hp;
	p.exp = clients[c_id]._exp;
	p.level = clients[c_id]._level;
	client.doSend(&p);
}

void CSendPacket::sendAttackPacket(CSession& client,int c_id, int skill_type , short active_type, short x , short y)
{
	SC_ATTACK_PACKET p;
	p.size = sizeof(SC_ATTACK_PACKET);
	p.id = c_id; 
	p.type = SC_ATTACK;
	p.skill_type = skill_type;
	p.active_type = active_type;
	p.x = x;
	p.y = y;

	client.doSend(&p);
}
