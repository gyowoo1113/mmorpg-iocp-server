#include "pch.h"
#include "Sector.h"


int nearDirectionX[9] = { -1,-1,-1,0,0,0,1,1,1 };
int nearDirectionY[9] = { -1,0,1,-1,0,1,-1,0,1 };
mutex global_sector_lock;

unordered_set<int> MakeNearList(int c_id)
{
	int h = W_HEIGHT / 10;
	int w = W_WIDTH / 10;

	unordered_set<int> new_near_list;

	for (int i = 0; i < 9; ++i)
	{
		int dirX = clients[c_id]._sector_x + nearDirectionX[i];
		int dirY = clients[c_id]._sector_y + nearDirectionY[i];

		if (dirX < 0 || dirY < 0 || dirX > w - 1 || dirY > h - 1) continue;

		for (auto id : sector[dirX][dirY])
		{
			if (c_id == id) continue;
			if (RANGE >= distance(c_id, id))
			{
				new_near_list.insert(id);
			}
		}
	}

	return (new_near_list);
}

void SetSector(int id)
{
	int x = (clients[id].x) / 10;
	int y = (clients[id].y) / 10;

	sector[x][y].insert(id);

	clients[id]._sector_x = x;
	clients[id]._sector_y = y;
}

void CheckMoveSector(int id)
{
	int cl_x = clients[id].x;
	int cl_y = clients[id].y;
	int sec_x = clients[id]._sector_x;
	int sec_y = clients[id]._sector_y;

	if (cl_x != sec_x || cl_y != sec_y)
		ChangeSector(id);
}

void ChangeSector(int id, bool update)
{
	int sec_x = clients[id]._sector_x;
	int sec_y = clients[id]._sector_y;

	global_sector_lock.lock();
	sector[sec_x][sec_y].unsafe_erase(id);
	global_sector_lock.unlock();
	if (update) SetSector(id);
}