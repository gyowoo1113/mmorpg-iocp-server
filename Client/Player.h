#pragma once
#include "stdafx.h"
#include "GameObject.h"

class CPlayer : public CGameObject
{
public:
	CPlayer(std::string id, int level, int hp, int exp)
		: m_id{ id }, m_level{ level }, m_hp{ hp }, m_exp{ exp } 
	{
		calculateMaxExp(); 
	};
	~CPlayer() {};

	void setParameter(sf::Text& text);
	void setLevelUp(int RemainExp);
	void getExp(int exp);
	void calculateMaxExp();
	string getId() { return m_id; }

private:
	std::string m_id;
	int m_level;
	int m_hp;
	int m_exp;
	int m_maxHp = 100;
	int m_maxExp;
};

