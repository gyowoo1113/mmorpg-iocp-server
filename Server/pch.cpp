#include "pch.h"
#include "Session.h"

std::list<USER_DATA> g_db_users;
concurrency::concurrent_unordered_set<int> sector[W_WIDTH / W_SECTOR][W_HEIGHT / H_SECTOR];
int tiles[W_WIDTH][W_HEIGHT] = {};
std::array<CSession, MAX_USER + NUM_NPC> clients;

int distance(int a, int b)
{
	return abs(clients[a].x - clients[b].x) + abs(clients[a].y - clients[b].y);
}

bool isMonsterCollisionNormalAttack(int& monster_id, int& p_id)
{
	int m_x = clients[monster_id].x;
	int m_y = clients[monster_id].y;
	int p_x = clients[p_id].x;
	int p_y = clients[p_id].y;

	if (p_x == m_x -1 && p_y == m_y) return true;
	if (p_x == m_x +1 && p_y == m_y) return true;
	if (p_x == m_x && p_y == m_y - 1) return true;
	if (p_x == m_x && p_y == m_y + 1) return true;

	return false;
}

bool isMonsterCollisionAttack(int& monster_id, int& p_id)
{
	int m_x = clients[monster_id].x;
	int m_y = clients[monster_id].y;
	int p_x = clients[p_id].x;
	int p_y = clients[p_id].y;

	if (p_x == m_x - 1 && p_y == m_y) return true;
	if (p_x == m_x + 1 && p_y == m_y) return true;
	if (p_x == m_x && p_y == m_y - 1) return true;
	if (p_x == m_x && p_y == m_y + 1) return true;

	if (p_x == m_x - 1 && p_y == m_y - 1) return true;
	if (p_x == m_x + 1 && p_y == m_y + 1) return true;
	if (p_x == m_x + 1 && p_y == m_y - 1) return true;
	if (p_x == m_x - 1 && p_y == m_y + 1) return true;

	return false;
}