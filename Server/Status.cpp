#include "pch.h"
#include "Status.h"
#include "Session.h"

bool CStatus::decreaseHp(CSession& client, int hp)
{
    client._hp -= hp;
    if (client._hp <= 0)
        return true;

    pair<int, int> id{ client._id,client._id };
    World::instance().addEvent(id, COMP_TYPE::EV_HEAL, 5000);

    return false;
}

void CStatus::healHp(CSession& client)
{
    client._hp += 10;
    if (client._hp < 100)
    {
        pair<int, int> id{ client._id,client._id };
        World::instance().addEvent(id, COMP_TYPE::EV_HEAL, 5000);
    }
}

void CStatus::updateExp(CSession& client, int mon_id)
{
    int mon_level = clients[mon_id]._level;
    int gainExp = mon_level * mon_level * 2;
    int mtype = clients[mon_id].monsterMoveType + 1;
    int type = clients[mon_id].monsterType + 1;

    gainExp *= mtype;
    gainExp *= type;

    client._exp += gainExp;
    if (client._maxExp <= client._exp)
        setLevelUp(client,client._exp - client._maxExp);

    string user(client._name), mons(clients[mon_id]._name);
    string mess = "User:" + user + " kill to " + mons + ", gain " + to_string(gainExp) + " Exp";
    client.chatSystemMessage(mess);
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

void CStatus::respawn(CSession& client)
{
    client._hp = 100;

    if (client._id >= MAX_USER) return;

    int exp = client._exp / 2;
    client._exp = exp;
}
