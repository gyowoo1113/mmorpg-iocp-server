#include "pch.h"
#include "Astar.h"

std::set<position> CAstar::closed;

void CAstar::initMapClosedList()
{
	for (int i = 0; i < W_WIDTH; ++i)
	{
		for (int j = 0; j < W_HEIGHT; ++j)
		{
			if (tiles[i][j])
			{
				position pos = std::make_pair(i, j);
				closed.insert(pos);
			}
		}
	}
}

bool compF(const sNode* lhs, const sNode* rhs) 
{
	return lhs->f < rhs->f;
};

void CAstar::calculateG(sNode* node, float& weight)
{
	if (node->pParent != nullptr)
	{
		node->g = node->pParent->g + weight;
	}
	else 
	{
		node->g = 0;
	}
}

void CAstar::calculateH(sNode* node)
{
	node->h = abs(node->pos.first - end_pos.first) + abs(node->pos.second - end_pos.second);
}

void CAstar::calculateF(sNode* node)
{
	node->f = node->g + node->h;
}

void CAstar::calculateScore(sNode* node, float& weight)
{
	calculateG(node, weight);
	calculateH(node);
	calculateF(node);
}

bool CAstar::searchRoad(short startX, short startY, short endX, short endY)
{
	start_pos = std::make_pair(startX, startY);
	end_pos = std::make_pair(endX, endY);
	initSearchLists();
	pEnd = nullptr;

	position now_pos;
	sNode* pop_node;
	int cnt = 0;
	while (true)
	{
		if (cnt++ > 50) return false;
		if (open.size() == 0) return false;

		open.sort(compF);
		pop_node = open.front();
		open.pop_front();
		now_pos = pop_node->pos;

		if (now_pos == end_pos) break;

		for (int i = 0; i < 8; i++) 
		{
			int x = now_pos.first + dx[i], y = now_pos.second + dy[i];
			if (x < 0 || y < 0) continue;
			if (x > W_WIDTH || y > W_HEIGHT) continue;

			auto p = closed.find(std::make_pair(x, y));
			if (p != closed.end()) continue;

			sNode* new_node = new sNode(std::make_pair(x, y), pop_node);
			calculateScore(new_node, weight[i]);
			open.push_back(new_node);
			compareG(new_node, i);
		}
	}

	pEnd = pop_node;

	return true;
}

void CAstar::initSearchLists()
{
	open.clear();

	sNode* opens = new sNode(start_pos);
	float s_weight = 0.0f;
	calculateScore(opens, s_weight);
	open.push_back(opens);
}

void CAstar::compareG(sNode* node, int dir)
{
	int _x = node->pos.first + dx[dir];
	int _y = node->pos.second + dy[dir];
	position pos = std::make_pair(_x, _y);

	auto iter = find_if(open.begin(), open.end(), [&pos](sNode* a) {
		if (a->pos == pos) return true;
		else return false;
	});

	if (iter != open.end())
	{
		(*iter)->pParent = node;
		calculateG((*iter), weight[dir]);
		calculateF((*iter));
	}
}

bool CAstar::getPathPosition(short* x, short* y)
{
	if (pEnd == nullptr) return false;
	sNode* checkNode = pEnd;

	if (checkNode == nullptr) return false;
	if (checkNode->pParent == nullptr) return false;
	if (checkNode->pParent->pos == start_pos) return true;

	checkNode = checkNode->pParent;
	*x = checkNode->pos.first;
	*y = checkNode->pos.second;
	return false;
}
