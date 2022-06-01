#include "stdafx.h"
#include "Effect.h"

CEffect::CEffect(sf::Texture& t, int x, int y, int x2, int y2, int frameCount)
	: CGameObject(t, x, y, x2, y2)
{
	setFrameCount(frameCount);
}

void CEffect::animDraw()
{
	if (false == m_bActive) return;

	if (m_nIdleIndex >= m_nFrameCount)
		initIndex();

	m_sprite.setTextureRect(sf::IntRect(m_nIdleIndex * 16, 0, 16, 16));

	CGameObject::draw();

	++m_nIdleIndex;
}

void CEffect::draw()
{
	if (false == m_bActive) return;

	m_sprite.setTextureRect(sf::IntRect(m_nIdleIndex * 16, 0, 16, 16));
	CGameObject::draw();
}

bool CEffect::isEndFrame()
{
	return (m_nIdleIndex >= m_nFrameCount);
}
