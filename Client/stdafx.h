#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <iostream>
#include <string>
#include <unordered_map>
#include <cmath>
using namespace std;

extern sf::RenderWindow* window;
extern int worldLeft;
extern int worldTop;
extern constexpr int CHAT_SIZE = 25;

extern constexpr auto SCREEN_WIDTH = 10;
extern constexpr auto SCREEN_HEIGHT = 10;
extern constexpr auto TILE_WIDTH = 64;
extern constexpr auto WINDOW_WIDTH = TILE_WIDTH * SCREEN_WIDTH;
extern constexpr auto WINDOW_HEIGHT = TILE_WIDTH * SCREEN_WIDTH + CHAT_SIZE;

constexpr int CHAT_LINE = 4;

extern int g_left_x;
extern int g_top_y;