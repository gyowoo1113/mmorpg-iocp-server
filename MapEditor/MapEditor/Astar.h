#pragma once
#include <set>
#include <list>
#include <vector>
#include <iostream>
using namespace std;

typedef pair<int, int> position;
extern int objects[100][100];

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
	bool searchDirections(vector<position>& road, int startX, int startY, int endX, int endY);
	void compareG(sNode* node, int dir);

protected:
	sNode* pStart = nullptr;
	sNode* pEnd = nullptr;
	set<position> closed;
	list<sNode*> open;

	position start_pos;
	position end_pos;
};