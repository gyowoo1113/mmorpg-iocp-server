#pragma once
#include "stdafx.h"

class CObject
{
public:
	virtual ~CObject() = 0;
	virtual void draw() = 0;
	virtual void animDraw() = 0;
};

class CGameObject : public CObject
{
protected:
	bool m_bActive = false;
	sf::Sprite m_sprite;
	int m_nIdleIndex = 0;
	int m_nStateIndex = 0;
	int m_nFrameCount = 0;
	int m_x = 0, m_y = 0;

public:
	CGameObject(sf::Texture& t, int x, int y, int x2, int y2) {
		m_bActive = false;
		m_sprite.setTexture(t);
		m_sprite.setTextureRect(sf::IntRect(x, y, x2, y2));
	}

	CGameObject() = default;

	void setActive(bool isActive);
	void spriteMove(int x, int y);
	void spriteDraw();
	void move(int x, int y);
	void setSpriteScale(float x, float y);
	void setSpriteRect(int x, int y, int x2, int y2);
	void setState(int index);
	void setFrameCount(int index);
	void initIndex();

	virtual void draw() override;
	virtual void animDraw() override;
};

