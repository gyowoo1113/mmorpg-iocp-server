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

	void show()
	{
		m_active = true;
	}

	void hide()
	{
		m_active = false;
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

	void draw() {
		if (false == m_active) return;

		float rx = (m_x - ::worldLeft) * (float)TILE_WIDTH + SCREEN_WIDTH;
		float ry = (m_y - ::worldTop) * (float)TILE_WIDTH + SCREEN_WIDTH;
		m_sprite.setPosition(rx, ry);
		window->draw(m_sprite);
	}
};

