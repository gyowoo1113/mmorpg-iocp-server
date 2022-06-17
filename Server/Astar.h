#pragma once
#include <set>
#include <queue>


typedef pair<short, short> position;

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
};

class CAstar
{
public:

	void calculateG(sNode* node, float& weight);
	void calculateH(sNode* node);
	void calculateF(sNode* node);
	void calculateScore(sNode* node, float& weight);
	bool searchRoad(short startX, short startY, short endX, short endY);
	void initSearchLists();
	void compareG(sNode* node, int dir);
	bool getPathPosition(short* x, short* y);

	static void initMapClosedList();

protected:
	sNode* pStart = nullptr;
	sNode* pEnd = nullptr;
	static set<position> closed;
	list<sNode*> open;
	stack<position> pathStack;

	position start_pos{ 0,0 };
	position end_pos{ 0,0 };

	int dx[8] = { 0,0,1,-1,1,-1,-1,1 };
	int dy[8] = { 1,-1,0,0,-1,1,-1,1 };
	float weight[8] = { 1.0f, 1.0f, 1.0f, 1.0f, 1.4f, 1.4f, 1.4f, 1.4f };
};