#pragma once
#include <set>
#include <queue>


int dx[8] = { 0,0,1,-1,1,-1,-1,1 };
int dy[8] = { 1,-1,0,0,-1,1,-1,1 };
float weight[8] = { 1.0f, 1.0f, 1.0f, 1.0f, 1.4f, 1.4f, 1.4f, 1.4f};

struct sNode {
	position pos;
	float g;
	int h;
	float f;
	sNode* pParent = nullptr;

	sNode(position p, sNode* par) : pos{ p }, pParent{ par }
	{

	}

	sNode(position p) : pos{ p } 
	{

	}

	bool operator<(const sNode& rhs) const
	{
		return f < rhs.f;
	}
};

class CAstar
{
public:

	void calculateG(sNode* node, float& weight);
	void calculateH(sNode* node);
	void calculateF(sNode* node);
	void calculateScore(sNode* node, float& weight);
	bool searchDirections(vector<position>& road,int startX, int startY, int endX, int endY);

protected:
	sNode* pStart = nullptr;
	sNode* pEnd = nullptr;
	set<position> closed;
	priority_queue<sNode*> open;

	position start_pos;
	position end_pos;
};

