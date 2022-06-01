#pragma once
#include "stdafx.h"
#include "GameObject.h"
#include "Effect.h"

class CPlayer : public CGameObject
{
public:
	CPlayer(sf::Texture& t, int x, int y, int x2, int y2, string id, int level, int hp, int exp);
	CPlayer();
	~CPlayer() = default;

	void setParameter(sf::Text& text);
	void setStatus(int hp, int level, int exp);
	void setAttack();
	string getName() { return m_sName; }
	float getExpRatio();

	void calculateMaxExp();

	virtual void draw() override;
	virtual void animDraw() override;
	void drawAttack();

private:
	int m_nExp;
	int m_nMaxHp = 100;
	int m_nMaxExp;
	bool m_bAttack = false;

	CEffect m_effectObject;
};

