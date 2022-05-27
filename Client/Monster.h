#pragma once
#include "stdafx.h"
#include "GameObject.h"

class CMonster : public CGameObject
{
public:
	CMonster(sf::Texture& t, int x, int y, int x2, int y2, std::string name, int level, int hp, int exp)
		: m_name{ name }, m_level{ level }, m_hp{ hp }, CGameObject(t, x, y, x2, y2)
	{
		setFrameCount(4);
	};
	CMonster()
	{
		m_level = 1;
		m_name = "";
		m_hp = 100;
		setFrameCount(4);
	}
	~CMonster() {};

	virtual void animDraw() {
		if (false == m_active) return;

		m_nIdleIndex = (m_nIdleIndex >= m_frameCount) ? 0 : ++m_nIdleIndex;
		m_sprite.setTextureRect(sf::IntRect(0, m_nIdleIndex * 16, 16, 16));

		CGameObject::draw();
	}

	void setHp(int hp) {
		m_hp = hp;
	}

private:
	int m_level;
	int m_hp;
	std::string m_name;
};

