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

	road.clear();
	int dx[8] = { 0,0,1,-1,1,-1,-1,1 };
	int dy[8] = { 1,-1,0,0,-1,1,-1,1 };
	float weight[8] = { 1.0f, 1.0f, 1.0f, 1.0f, 1.5f, 1.5f, 1.5f, 1.5f };


	int W_WIDTH = 100;
	int W_HEIGHT = 100;

	cout << "sX: " << startX << " sY:" << startY << endl;
	cout << "eX: " << endX << " sY:" << endY << endl;


	start_pos = make_pair(startX, startY);
	end_pos = make_pair(endX, endY);

	sNode* opens = new sNode(start_pos);
	float s_weight = 0.0f;
	calculateScore(opens, s_weight);
	open.push_back(opens);

	cout << opens->h << endl;
	cout << opens->f << endl;
	cout << endl;

	for (int i = 0; i < W_WIDTH; ++i)
	{
		for (int j = 0; j < W_HEIGHT; ++j)
		{
			if (objects[i][j])
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

		open.sort(compF);
		pop_node = open.front();
		open.pop_front();
		now_pos = pop_node->pos;
		if (now_pos == end_pos) break;

		//cout << "¼±ÅÃ: " << now_pos.first << " " << now_pos.second << " : " << pop_node->f << endl;


		for (int i = 0; i < 8; i++) {
			int x = now_pos.first + dx[i], y = now_pos.second + dy[i];
			if (x < 0 || y < 0) continue;
			if (x > 100 || y > 100) continue;

			auto p = closed.find(make_pair(x, y));
			if (p != closed.end()) continue;

			sNode* new_node = new sNode(make_pair(x, y), pop_node);
			calculateScore(new_node, weight[i]);
			open.push_back(new_node);
			//cout << new_node->pos.first << " " << new_node->pos.second << " : " << new_node->f << endl;

			compareG(new_node, i);
		}

		closed.insert(pop_node->pos);
	}

	for (sNode* node = pop_node; node->pParent != nullptr; node = node->pParent)
	{
		cout << node->pos.first << " " << node->pos.second << endl;
		road.push_back(node->pos);
	}
	return true;
}

void CAstar::compareG(sNode* node, int dir)
{
	int dx[8] = { 0,0,1,-1,1,-1,-1,1 };
	int dy[8] = { 1,-1,0,0,-1,1,-1,1 };
	float weight[8] = { 1.0f, 1.0f, 1.0f, 1.0f, 1.4f, 1.4f, 1.4f, 1.4f };

	int _x = node->pos.first + dx[dir];
	int _y = node->pos.second + dy[dir];
	position pos = make_pair(_x, _y);

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
