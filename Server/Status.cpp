#include "pch.h"
#include "Status.h"
#include "Session.h"

bool CStatus::decreaseHp(CSession& client, int hp)
{
    client._hp -= hp;
    if (client._hp <= 0)
        return true;

    return false;
}

void CStatus::healHp(CSession& client)
{
    client._hp += 10;
    if (client._hp < 100)
    {
    }
}

void CStatus::updateExp(CSession& client)
{
    int mon_level = client._level;
    client._exp += mon_level * mon_level * 2;
    if (client._maxExp <= client._exp)
        setLevelUp(client,client._exp - client._maxExp);
}

void CStatus::calculateMaxExp(CSession& client)
{
    client._maxExp = pow(2, client._level - 1) * 100;
}

void CStatus::setLevelUp(CSession& client, int remainExp)
{
    ++client._level;
    client._maxExp = client._maxExp * 2;
    client._exp = remainExp;
    calculateMaxExp(client);
}
