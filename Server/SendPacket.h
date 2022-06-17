#pragma once

class CSession;
class CSendPacket
{
public:
	void send_login_fail(CSession& client);
	void send_login_info_packet(CSession& client);
	void send_move_packet(CSession& client, int c_id, int client_time);
	void send_add_object(CSession& client, int c_id);
	void send_remove_object(CSession& client, int c_id);
	void send_chat_packet(CSession& client, int c_id, const char* mess);
	void send_change_status_packet(CSession& client, int c_id);
	void send_attack_packet(CSession& client,int c_id , int skill_type , short x, short y);
};

