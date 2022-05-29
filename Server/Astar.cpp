#include "pch.h"
#include "Astar.h"

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
