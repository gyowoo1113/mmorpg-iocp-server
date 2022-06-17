#include "stdafx.h"
#include "Player.h"

CPlayer::CPlayer(sf::Texture& t, sf::Texture& et, int x, int y, int x2, int y2, string id, int level, int hp, int exp)
: m_nExp{ exp }, CGameObject(t, x, y, x2, y2, id, level, hp)
{
	calculateMaxExp();

	m_effectObject = CEffect(et, 0, 0, 16, 16, 4);
	m_effectObject.setSpriteScale(2.0f, 2.0f);
	m_effectObject.setActive(true);
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

void CPlayer::setAttack()
{
	m_bAttack = true;
}

float CPlayer::getExpRatio()
{
	return (float)m_nExp / (float)m_nMaxExp;
}

void CPlayer::calculateMaxExp()
{
	m_nMaxExp = pow(2, m_nLevel - 1) * 100;
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

void CPlayer::drawAttack()
{
	if (m_bAttack == false) return;

	int dx[4] = { -1,1,0,0 };
	int dy[4] = { 0,0,-1,1 };

	for (int i = 0; i < 4; ++i)
	{
		m_effectObject.move(m_x + dx[i], m_y + dy[i]);
		m_effectObject.draw();
	}

	m_effectObject.updateIndex();

	if (m_effectObject.isEndFrame())
	{
		m_bAttack = false;
		m_effectObject.initIndex();
	}
}