#pragma once
#include "stdafx.h"
#include "GameObject.h"
#include "Effect.h"

class CPlayer : public CGameObject
{
public:
	CPlayer(sf::Texture& t, sf::Texture& et, int x, int y, int x2, int y2, string id, int level, int hp, int exp);
	CPlayer();
	~CPlayer() = default;

	void setParameter(sf::Text& text);
	void setStatus(int hp, int level, int exp);
	void setAttack(int index);
	string getName() { return m_sName; }
	float getExpRatio();
	void normalAttackDraw();
	void calculateMaxExp();

	bool isCoolDown(int index);
	void activeCoolDown(int index);

	virtual void draw() override;
	virtual void animDraw() override;
	virtual void drawAttack(int index) override;
	virtual void drawAttacks() override;

private:
	int m_nExp;
	int m_nMaxHp = 100;
	int m_nMaxExp;
	bool _isDrawAttack[3] = { false, false, false };
	bool setLoop[3] = { false,false,true };
	bool _isCoolDown[3] = { true,true,true };
	CEffect m_effectObject[3];
	vector<vector<int>> dx;
	vector<vector<int>> dy;
};
