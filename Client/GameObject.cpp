#include "GameObject.h"
#include "stdafx.h"

void CGameObject::setActive(bool isActive)
{
	m_bActive = isActive;
}

void CGameObject::spriteMove(int x, int y)
{
	m_sprite.setPosition((float)x, (float)y);
}

void CGameObject::spriteDraw()
{
	window->draw(m_sprite);
}

void CGameObject::move(int x, int y)
{
	m_x = x;
	m_y = y;
}

void CGameObject::setSpriteScale(float x, float y)
{
	m_sprite.setScale(sf::Vector2f(x, y));
}

void CGameObject::setSpriteRect(int x, int y, int x2, int y2)
{
	m_sprite.setTextureRect(sf::IntRect(x, y, x2, y2));
}

void CGameObject::setState(int index)
{
	m_nStateIndex = index;
}

void CGameObject::setFrameCount(int index)
{
	m_nFrameCount = index - 1;
}

void CGameObject::initIndex()
{
	m_nIdleIndex = 0;
}

// ** ** //

void CGameObject::draw()
{
	if (false == m_bActive) return;

	float rx = (m_x - ::g_left_x) * (float)TILE_WIDTH;
	float ry = (m_y - ::g_top_y) * (float)TILE_WIDTH;
	m_sprite.setPosition(rx, ry);
	window->draw(m_sprite);
}

void CGameObject::animDraw()
{
}
