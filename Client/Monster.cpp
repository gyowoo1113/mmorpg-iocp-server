#include "stdafx.h"
#include "Monster.h"

CMonster::CMonster(sf::Texture& t, int x, int y, int x2, int y2, std::string name, int level, int hp, int exp)
	: CGameObject(t, x, y, x2, y2, name,level,exp)
{
	setFrameCount(4);
}

CMonster::CMonster()
{
	setFrameCount(4);
}

void CMonster::animDraw()
{
	if (false == m_bActive) return;

	m_nIdleIndex = (m_nIdleIndex >= m_nFrameCount) ? 0 : ++m_nIdleIndex;
	m_sprite.setTextureRect(sf::IntRect(0, m_nIdleIndex * 16, 16, 16));

	CGameObject::draw();
}
