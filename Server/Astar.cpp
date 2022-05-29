#include "pch.h"
#include "Astar.h"

bool compF(const sNode* lhs, const sNode* rhs) {
	return lhs->f < rhs->f;
};

void CAstar::calculateG(sNode* node, float& weight)
{
	if (node->pParent != nullptr)
		node->g = node->pParent->g + weight;
	else
		node->g = 0;
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

bool CAstar::searchDirections(vector<position>& road, int startX, int startY, int endX, int endY)
{
	start_pos = make_pair(startX, startY);
	end_pos = make_pair(endX, endY);

	road.push_back(start_pos);

	sNode* opens = new sNode(start_pos);
	float s_weight = 0.0f;
	calculateScore(opens,s_weight);
	open.push_back(opens);

	for (int i = 0; i < W_WIDTH; ++i)
	{
		for (int j = 0; j < W_HEIGHT; ++j)
		{
			if (tiles[i][j])
			{
				position pos = make_pair(i, j);
				closed.insert(pos);
			}
		}
	}

	position now_pos;
	sNode* pop_node;
	while (true)
	{
		if (open.size() == 0) return false;
		if (road.back() == end_pos) return true;

		open.sort(compF);
		pop_node = open.front();
		open.pop_front();
		now_pos = pop_node->pos;

		for (int i = 0; i < 8; i++) {
			int x = now_pos.first + dx[i], y = now_pos.second + dy[i];
			if (x < 0 || y < 0) continue;

			auto p = closed.find(make_pair(x, y));
			if (p != closed.end()) continue;

			sNode* new_node = new sNode(make_pair(x, y), pop_node);
			calculateScore(new_node, weight[i]);
			open.push_back(new_node);
		}
	}
}
