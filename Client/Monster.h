#pragma once
#include "stdafx.h"
#include "GameObject.h"
#include "Effect.h"

class CMonster : public CGameObject
{
public:
	CMonster(sf::Texture& t, sf::Texture& et, int x, int y, int x2, int y2, std::string name, int level, int hp, int exp);
	CMonster();
	~CMonster() = default;

	virtual void animDraw() override;
	virtual void drawAttack() override;
	void setAttack();
	void effectMove(int x, int y);

private:
	bool m_bAttack = false;
	CEffect m_effectObject;
	int m_ex, m_ey;
};

