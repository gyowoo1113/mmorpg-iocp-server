#include "stdafx.h"
#include "GameObject.h"

CGameObject::CGameObject(sf::Texture& t, int x, int y, int x2, int y2)
{
	m_sprite.setTexture(t);
	m_sprite.setTextureRect(sf::IntRect(x, y, x2, y2));
}

CGameObject::CGameObject(sf::Texture& t, int x, int y, int x2, int y2, string name, int level, int hp)
: m_sName{ name }, m_nLevel{ level }, m_nHp{ hp }
{
	m_sprite.setTexture(t);
	m_sprite.setTextureRect(sf::IntRect(x, y, x2, y2));
	setNameUI(name.c_str());
}

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

void CGameObject::setHp(int hp)
{
	m_nHp = hp;
}

void CGameObject::initIndex()
{
	m_nIdleIndex = 0;
}

void CGameObject::setNameUI(const char name[])
{
	nameUI.setFont(g_font);
	nameUI.setString(name);
	nameUI.setFillColor(sf::Color(0,0,200));
	nameUI.setCharacterSize(15);
	nameUI.setStyle(sf::Text::Bold);
}

// ** ** //

void CGameObject::draw()
{
	if (false == m_bActive) return;

	float rx = (m_x - ::g_left_x) * (float)TILE_WIDTH;
	float ry = (m_y - ::g_top_y) * (float)TILE_WIDTH;
	m_sprite.setPosition(rx, ry);
	window->draw(m_sprite);

	nameUI.setPosition(rx - 10, ry - 20);
	window->draw(nameUI);
}

void CGameObject::animDraw()
{
}

void CGameObject::drawAttack()
{
}

void CGameObject::drawAttacks()
{
}

void CGameObject::drawAttack(int index)
{
}
