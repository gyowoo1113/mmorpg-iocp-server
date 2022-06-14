#pragma once

class World
{
public:
	static World& instance();
	SOCKET& getServerSocket() { return server_socket; }
	HANDLE& getHandle() { return handle_iocp; }

	int get_new_client_id();
	void disconnect(int c_id);

	void initialize_tilemap();
	void initialize_npc();

	void process_work(OVER_EXP* ex_over, DWORD& num_bytes, ULONG_PTR& key);
	void accept_client(OVER_EXP* ex_over, DWORD& num_bytes, ULONG_PTR& key);
	void recv_client(OVER_EXP* ex_over, DWORD& num_bytes, ULONG_PTR& key);
	void send_client(OVER_EXP* ex_over, DWORD& num_bytes, ULONG_PTR& key);

private:
	static World _instance;
	static bool _instantiated;
	SOCKET server_socket;
	HANDLE handle_iocp;
};

