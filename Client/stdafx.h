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
extern constexpr int CHAT_SIZE = 20;

extern constexpr auto SCREEN_WIDTH = 10;
extern constexpr auto SCREEN_HEIGHT = 10;
extern constexpr auto TILE_WIDTH = 65;
extern constexpr auto WINDOW_WIDTH = TILE_WIDTH * SCREEN_WIDTH + 10;
extern constexpr auto WINDOW_HEIGHT = TILE_WIDTH * SCREEN_WIDTH + 10;

constexpr int W_WIDTH = 20;
constexpr int W_HEIGHT = 20;

