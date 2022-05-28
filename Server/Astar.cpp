#include "pch.h"
#include "Astar.h"

void CAstar::calculateG(sNode* node, float& weight)
{
	node->g = node->pParent->g + weight;
}

void CAstar::calculateH(sNode* node)
{
	if (pEnd == nullptr) return;
	node->h = abs(node->pos.first - pEnd->pos.first) + abs(node->pos.second - pEnd->pos.second);
}

void CAstar::calculateF(sNode* node)
{
	node->f = node->g + node->h;
}

void CAstar::calculateWeight(sNode* node, float& weight)
{
	calculateG(node, weight);
	calculateH(node);
	calculateF(node);
}
