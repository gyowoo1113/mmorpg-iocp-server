#pragma once
#include "stdafx.h"

class CObject
{
public:
	virtual void draw() = 0;
	virtual void animDraw() = 0;
	virtual void drawAttack() = 0;
};

class CGameObject : public CObject
{
protected:
	bool m_bActive = false;
	sf::Sprite m_sprite;
	sf::Text nameUI;

	int m_nIdleIndex = 0;
	int m_nStateIndex = 0;
	int m_nFrameCount = 0;
	int m_x = 0, m_y = 0;
	int m_nLevel = 1;
	int m_nHp = 100;
	string m_sName = "";

public:
	CGameObject(sf::Texture& t, int x, int y, int x2, int y2);
	CGameObject(sf::Texture& t, int x, int y, int x2, int y2, string name, int level, int hp);
	CGameObject() = default;

	void setActive(bool isActive);
	void spriteMove(int x, int y);
	void spriteDraw();
	void move(int x, int y);
	void setSpriteScale(float x, float y);
	void setSpriteRect(int x, int y, int x2, int y2);
	void setState(int index);
	void setFrameCount(int index);
	void setHp(int hp);
	void initIndex();
	void setNameUI(const char name[]);
	int getX() { return m_x; }
	int getY() { return m_y; }

	virtual void draw() override;
	virtual void animDraw() override;
	virtual void drawAttack() override;
};

