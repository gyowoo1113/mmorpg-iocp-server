#include "pch.h"
#include "Sector.h"


std::mutex global_sector_lock;

void setSector(int id)
{
	int x = (clients[id].x) / W_SECTOR;
	int y = (clients[id].y) / H_SECTOR;

	sector[x][y].insert(id);

	clients[id]._sector_x = x;
	clients[id]._sector_y = y;
}

void checkMoveSector(int id)
{
	int cl_x = clients[id].x;
	int cl_y = clients[id].y;
	int sec_x = clients[id]._sector_x;
	int sec_y = clients[id]._sector_y;

	if (cl_x != sec_x || cl_y != sec_y)
		changeSector(id);
}

void changeSector(int id, bool update)
{
	int sec_x = clients[id]._sector_x;
	int sec_y = clients[id]._sector_y;

	global_sector_lock.lock();
	sector[sec_x][sec_y].unsafe_erase(id);
	global_sector_lock.unlock();
	if (update) setSector(id);
}