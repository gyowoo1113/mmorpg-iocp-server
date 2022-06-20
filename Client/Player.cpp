#include "stdafx.h"
#include "Player.h"

CPlayer::CPlayer(sf::Texture& t, sf::Texture& et, int x, int y, int x2, int y2, string id, int level, int hp, int exp)
: m_nExp{ exp }, CGameObject(t, x, y, x2, y2, id, level, hp)
{
	calculateMaxExp();

	m_effectObject[0] = CEffect(et, 0, 0, 16, 16, 4);
	m_effectObject[0].setSpriteScale(2.0f, 2.0f);
	m_effectObject[0].setActive(true);

	m_effectObject[1] = CEffect(*effectTiles[2], 0, 0, 16, 16, 4);
	m_effectObject[1].setSpriteScale(6.0f, 6.0f);
	m_effectObject[1].setActive(true);

	m_effectObject[2] = CEffect(*effectTiles[3], 0, 0, 16, 16, 4);
	m_effectObject[2].setSpriteScale(2.0f, 2.0f);
	m_effectObject[2].setActive(true);
}

CPlayer::CPlayer()
{
	calculateMaxExp();
}

void CPlayer::setParameter(sf::Text& text)
{
	string pos = "(" + to_string(m_x) + "," + to_string(m_y) + ")    ";
	string id = "ID:" + m_sName + " ";
	string level = "LV:" + to_string(m_nLevel) + " ";
	string hp = "HP:" + to_string(m_nHp) + "/" + to_string(m_nMaxHp) + " ";
	string exp = "EXP: " + to_string(m_nExp) + "/" + to_string(m_nMaxExp);

	string val = pos + id + level + hp + exp;
	text.setString(val);
}

void CPlayer::setStatus(int hp, int level, int exp)
{
	m_nHp = hp;
	m_nExp = exp;

	if (m_nLevel != level)
	{
		m_nLevel = level;
		calculateMaxExp();
	}
}

void CPlayer::setAttack(int index)
{
	_isDrawAttack[index] = true;
	_isCoolDown[index] = false;
}

float CPlayer::getExpRatio()
{
	return (float)m_nExp / (float)m_nMaxExp;
}

void CPlayer::normalAttackDraw()
{
	int dx[4] = { -1,1,0,0 };
	int dy[4] = { 0,0,-1,1 };

	for (int i = 0; i < 4; ++i)
	{
		m_effectObject[0].move(m_x + dx[i], m_y + dy[i]);
		m_effectObject[0].draw();
	}
}

void CPlayer::calculateMaxExp()
{
	m_nMaxExp = pow(2, m_nLevel - 1) * 100;
}


bool CPlayer::isCoolDown(int index)
{
	return _isCoolDown[index];
}

void CPlayer::activeCoolDown(int index)
{
	_isCoolDown[index] = true;
}

void CPlayer::draw()
{
	if (m_bActive == false) return;

	m_sprite.setTextureRect(sf::IntRect(m_nStateIndex * 16, 0, 16, 16));

	CGameObject::draw();
}

void CPlayer::animDraw()
{
	if (m_bActive == false) return;

	m_nIdleIndex = (m_nIdleIndex >= m_nFrameCount) ? 0 : ++m_nIdleIndex;
	m_sprite.setTextureRect(sf::IntRect(m_nStateIndex * 16, m_nIdleIndex * 16, 16, 16));

	CGameObject::draw();
}

void CPlayer::drawAttack(int index)
{
	if (_isDrawAttack[index] == false) return;

	int dxy;
	if (index == 0 ) normalAttackDraw();
	else {
		dxy = (index == 1) ? -1 : 0;
		m_effectObject[index].move(m_x + dxy, m_y + dxy);
		m_effectObject[index].draw();
	}
	
	m_effectObject[index].updateIndex();

	if (m_effectObject[index].isEndFrame())
	{
		if (!setLoop[index]) _isDrawAttack[index] = false;
		m_effectObject[index].initIndex();
	}
}

void CPlayer::drawAttacks()
{
	for (int i = 0; i < 3; ++i)
		drawAttack(i);
}
