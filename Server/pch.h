#pragma once
#include "../Client/protocol.h"
#include <SFML/Graphics.hpp>

#include <iostream>
#include <WS2tcpip.h>
#include <MSWSock.h>

#include <thread>
#include <array>
#include <vector>
#include <mutex>
#include <string>
#include <unordered_set>
#include <fstream>
#include "Session.h"

extern constexpr int RANGE = 5;
class CSession;

extern unordered_set<int> sector[W_WIDTH / 10][W_HEIGHT / 10];
extern int tiles[W_WIDTH][W_HEIGHT];
extern mutex secl;
extern array<CSession, MAX_USER + NUM_NPC> clients;

int distance(int a, int b);
void disconnect(int c_id);
void update_move_clients(int c_id, char& direction);
bool isMonsterCollisionAttack(int& monster_id, int& p_id);
void process_packet(int c_id, char* packet);

#include "Sector.h"