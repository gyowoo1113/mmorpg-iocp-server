#include "stdafx.h"
#include "Monster.h"

void CMonster::animDraw()
{
	if (false == m_bActive) return;

	m_nIdleIndex = (m_nIdleIndex >= m_nFrameCount) ? 0 : ++m_nIdleIndex;
	m_sprite.setTextureRect(sf::IntRect(0, m_nIdleIndex * 16, 16, 16));

	CGameObject::draw();
}

void CMonster::setHp(int hp)
{
	m_nHp = hp;
}
