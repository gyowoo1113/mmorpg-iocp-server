#pragma once
#include "stdafx.h"
#include "GameObject.h"

class CEffect : public CGameObject
{
public:
	CEffect(sf::Texture& t, int x, int y, int x2, int y2, int frameCount);
	CEffect() = default;

	virtual void animDraw() override;
	virtual void draw() override;

	void updateIndex();
	bool isEndFrame();
};

