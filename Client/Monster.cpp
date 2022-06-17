#include "stdafx.h"
#include "Monster.h"

CMonster::CMonster(sf::Texture& t, sf::Texture& et, int x, int y, int x2, int y2, std::string name, int level, int hp, int exp)
	: CGameObject(t, x, y, x2, y2, name,level,exp)
{
	setFrameCount(4);

	m_effectObject = CEffect(et, 0, 0, 16, 16, 5);
	m_effectObject.setSpriteScale(2.0f, 2.0f);
	m_effectObject.setActive(true);
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

void CMonster::drawAttack()
{
	if (m_bAttack == false) return;

	m_effectObject.move(m_ex, m_ey);
	m_effectObject.draw();
	m_effectObject.updateIndex();

	if (m_effectObject.isEndFrame())
	{
		m_bAttack = false;
		m_effectObject.initIndex();
	}
}

void CMonster::effectMove(int x, int y)
{
	m_ex = x;
	m_ey = y;
}

void CMonster::setAttack()
{
	m_bAttack = true;
}
