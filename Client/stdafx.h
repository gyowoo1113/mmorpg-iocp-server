#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <iostream>
#include <string>
#include <unordered_map>

extern sf::RenderWindow* window;
extern int worldLeft;
extern int worldTop;

extern constexpr auto SCREEN_WIDTH = 10;
extern constexpr auto SCREEN_HEIGHT = 10;
extern constexpr auto TILE_WIDTH = 65;
extern constexpr auto WINDOW_WIDTH = TILE_WIDTH * SCREEN_WIDTH + 10;
extern constexpr auto WINDOW_HEIGHT = TILE_WIDTH * SCREEN_WIDTH + 10;