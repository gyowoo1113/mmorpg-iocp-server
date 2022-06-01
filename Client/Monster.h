#pragma once
#include "stdafx.h"
#include "GameObject.h"

class CMonster : public CGameObject
{
public:
	CMonster(sf::Texture& t, int x, int y, int x2, int y2, std::string name, int level, int hp, int exp)
		: m_sName{ name }, m_nLevel{ level }, m_nHp{ hp }, CGameObject(t, x, y, x2, y2);

	CMonster()
	{
		setFrameCount(4);
	}
	~CMonster() {};

	virtual void animDraw() override;
	void setHp(int hp);

private:
	int m_nLevel = 1;
	int m_nHp = 100;
	std::string m_sName = "";
};

