#pragma once

class CSession;
class CSendPacket
{
public:
	void sendLoginFail(CSession& client);
	void sendLoginInfoPacket(CSession& client);
	void sendMovePacket(CSession& client, int c_id, int client_time);
	void sendAddObject(CSession& client, int c_id);
	void sendRemoveObject(CSession& client, int c_id);
	void sendChatPacket(CSession& client, int c_id, const char* mess);
	void sendChangeStatusPacket(CSession& client, int c_id);
	void sendAttackPacket(CSession& client,int c_id , int skill_type , short active_type, short x, short y);
};

