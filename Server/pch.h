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
extern mutex secl;
extern array<CSession, MAX_USER + NUM_NPC> clients;

int distance(int a, int b);

#include "Sector.h"