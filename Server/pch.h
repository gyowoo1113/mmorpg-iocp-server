#pragma once
#include "../Client/protocol.h"
#include <SFML/Graphics.hpp>

#include <iostream>
#include <WS2tcpip.h>
#include <MSWSock.h>

#include <unordered_map>
#include <cmath>
#include <thread>
#include <array>
#include <vector>
#include <mutex>
#include <string>
#include <unordered_set>
#include <fstream>
#include <stack>
#include <chrono>
#include <concurrent_unordered_set.h>
#include <list>
#include "Session.h"

extern constexpr int RANGE = 7;
extern constexpr int MONSTER_RANGE = 5;
class CSession;
struct USER_DATA
{
	int x, y;
	char name[20];
	int level, exp;
	int hp;
};

extern constexpr int W_SECTOR = 50;
extern constexpr int H_SECTOR = 50;

extern std::list<USER_DATA> g_db_users;
extern concurrency::concurrent_unordered_set<int> sector[W_WIDTH / W_SECTOR][W_HEIGHT / H_SECTOR];
extern int tiles[W_WIDTH][W_HEIGHT];
extern std::array<CSession, MAX_USER + NUM_NPC> clients;

int distance(int a, int b);
void updateMoveClients(int c_id, char& direction);
bool isMonsterCollisionNormalAttack(int& monster_id, int& p_id);
bool isMonsterCollisionAttack(int& monster_id, int& p_id);
void moveNpc(int npc_id);

#include "Sector.h"
#include "Database.h"
#include "Astar.h"
#include "World.h"