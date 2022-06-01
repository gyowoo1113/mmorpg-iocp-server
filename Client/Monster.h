#pragma once
#include "stdafx.h"
#include "GameObject.h"

class CMonster : public CGameObject
{
public:
	CMonster(sf::Texture& t, int x, int y, int x2, int y2, std::string name, int level, int hp, int exp);
	CMonster();
	~CMonster() = default;

	virtual void animDraw() override;
};

