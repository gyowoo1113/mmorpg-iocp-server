#include "pch.h"
#include "Session.h"

unordered_set<int> sector[W_WIDTH / 10][W_HEIGHT / 10] = {};
mutex secl;
array<CSession, MAX_USER + NUM_NPC> clients;

int distance(int a, int b)
{
	return abs(clients[a].x - clients[b].x) + abs(clients[a].y - clients[b].y);
}