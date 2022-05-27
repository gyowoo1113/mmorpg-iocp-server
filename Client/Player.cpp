#include "stdafx.h"
#include "Player.h"

void CPlayer::setParameter(sf::Text& text)
{
	string pos = "(" + to_string(m_x) + "," + to_string(m_y) + ")    ";
	string id = "ID:" + m_id + " ";
	string level = "LV:" + to_string(m_level) + " ";
	string hp = "HP:" + to_string(m_hp) + "/" + to_string(m_maxHp) + " ";
	string exp = "EXP: " + to_string(m_exp) + "/" + to_string(m_maxExp);

	string val = pos + id + level + hp + exp;
	text.setString(val);
}

void CPlayer::setLevelUp(int remainExp)
{
	++m_level;
	m_maxExp *= 2;
	m_exp = remainExp;
	calculateMaxExp();
}

void CPlayer::getExp(int exp)
{
	m_exp += exp;
	updateExp();
}

void CPlayer::updateExp()
{
	if (m_maxExp <= m_exp)
		setLevelUp(m_exp - m_maxExp);
}

void CPlayer::calculateMaxExp()
{
	m_maxExp = pow(2, m_level - 1) * 100;
}

void CPlayer::setStatus(int hp, int level, int exp)
{
	m_hp = hp;
	m_exp = exp;

	if (m_level != level)
	{
		m_level = level;
		calculateMaxExp();
	}
}
