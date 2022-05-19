#pragma once
#include "stdafx.h"

class CGameObject
{
private:
	bool m_active;
	sf::Sprite m_sprite;

public:
	int m_x, m_y;
	CGameObject(sf::Texture& t, int x, int y, int x2, int y2) {
		m_active = false;
		m_sprite.setTexture(t);
		m_sprite.setTextureRect(sf::IntRect(x, y, x2, y2));
	}

	CGameObject() 
	{
		m_active = false;
	}

	void setActive(bool isActive)
	{
		m_active = isActive;
	}

	void spriteMove(int x, int y) {
		m_sprite.setPosition((float)x, (float)y);
	}

	void spriteDraw() {
		window->draw(m_sprite);
	}

	void move(int x, int y) {
		m_x = x;
		m_y = y;
	}

	void setSpriteScale(float x, float y) {
		m_sprite.setScale(sf::Vector2f(x, y));
	}

	void setSpriteRect(int x, int y, int x2, int y2){
		m_sprite.setTextureRect(sf::IntRect(x, y, x2, y2));
	}

	void draw() {
		if (false == m_active) return;

		float rx = (m_x - ::g_left_x) * (float)TILE_WIDTH;
		float ry = (m_y - ::g_top_y) * (float)TILE_WIDTH;
		m_sprite.setPosition(rx, ry);
		window->draw(m_sprite);
	}
};

