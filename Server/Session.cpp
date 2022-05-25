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