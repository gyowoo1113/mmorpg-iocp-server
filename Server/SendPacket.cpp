#include "pch.h"
#include "SendPacket.h"
#include "Session.h"

void CSendPacket::send_login_fail(CSession& client)
{
	SC_LOGIN_FAIL_PACKET p;
	p.size = sizeof(SC_LOGIN_FAIL_PACKET);
	p.type = SC_LOGIN_FAIL;
	client.do_send(&p);
}

void CSendPacket::send_login_info_packet(CSession& client)
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
	client.do_send(&p);
}

void CSendPacket::send_move_packet(CSession& client, int c_id, int client_time)
{
	SC_MOVE_OBJECT_PACKET p;
	p.id = c_id;
	p.size = sizeof(SC_MOVE_OBJECT_PACKET);
	p.type = SC_MOVE_OBJECT;
	p.x = clients[c_id].x;
	p.y = clients[c_id].y;
	p.client_time = client_time;
	client.do_send(&p);
}

void CSendPacket::send_add_object(CSession& client, int c_id)
{
	SC_ADD_OBJECT_PACKET p;
	p.id = c_id;
	p.size = sizeof(SC_ADD_OBJECT_PACKET);
	p.type = SC_ADD_OBJECT;
	p.x = clients[c_id].x;
	p.y = clients[c_id].y;
	p.race = clients[c_id].monsterType * 2 +  clients[c_id].monsterMoveType;
	strcpy_s(p.name, clients[c_id]._name);
	client.do_send(&p);
}

void CSendPacket::send_remove_object(CSession& client, int c_id)
{
	SC_REMOVE_OBJECT_PACKET p;
	p.id = c_id;
	p.size = sizeof(SC_REMOVE_OBJECT_PACKET);
	p.type = SC_REMOVE_OBJECT;
	client.do_send(&p);
}

void CSendPacket::send_chat_packet(CSession& client, int c_id, const char* mess)
{
	SC_CHAT_PACKET p;
	p.id = c_id;
	p.size = sizeof(SC_CHAT_PACKET) - sizeof(p.mess) + strlen(mess) + 1;
	p.type = SC_CHAT;
	strcpy_s(p.mess, mess);
	client.do_send(&p);
}

void CSendPacket::send_change_status_packet(CSession& client, int c_id)
{
	SC_STAT_CHANGE_PACKET p;
	p.size = sizeof(SC_STAT_CHANGE_PACKET);
	p.type = SC_STAT_CHANGE;
	p.id = c_id;
	p.hp = clients[c_id]._hp;
	p.exp = clients[c_id]._exp;
	p.level = clients[c_id]._level;
	client.do_send(&p);
}
