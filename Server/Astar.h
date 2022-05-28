#pragma once
#include <set>

typedef pair<int, int> position;

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
	void calculateWeight(sNode* node, float& weight);

protected:
	sNode* pStart = nullptr;
	sNode* pEnd = nullptr;
	set<sNode> closed;
	set<sNode> open;
};

