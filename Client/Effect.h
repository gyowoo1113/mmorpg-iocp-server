#pragma once
#include "stdafx.h"
#include "GameObject.h"

class CEffect : public CGameObject
{
public:
	CEffect(sf::Texture& t, int x, int y, int x2, int y2, int frameCount) : CGameObject(t, x, y, x2, y2)
	{
		setFrameCount(frameCount);
	}
	CEffect()
	{

	}

	virtual void animDraw() {
		if (false == m_active) return;

		if (m_nIdleIndex >= m_frameCount)
			initEffect();

		m_sprite.setTextureRect(sf::IntRect(m_nIdleIndex * 16, 0 , 16, 16));

		CGameObject::draw();

		++m_nIdleIndex;
	}

	virtual void draw() {
		if (false == m_active) return;

		m_sprite.setTextureRect(sf::IntRect(m_nIdleIndex * 16, 0, 16, 16));
		CGameObject::draw();
	}

	void updateIndex() {
		++m_nIdleIndex;
	}

	void initEffect()
	{
		m_nIdleIndex = 0;
		//m_active = false;
	}

	bool isEndFrame()
	{
		return (m_nIdleIndex >= m_frameCount);
	}
};

