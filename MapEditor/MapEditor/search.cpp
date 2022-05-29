#include<iostream>
#include<fstream>
#include "Astar.h"

using namespace std;


int objects[100][100] = {};

int main()
{

	int W_WIDTH = 100;
	int W_HEIGHT = 100;

	for (int i = 0; i < W_WIDTH; ++i)
	{
		for (int j = 0; j < W_HEIGHT; ++j)
		{
			switch (rand() % 20)
			{
			//case 1:
			//	objects[i][j] = 1;
			//	break;

			default:
				objects[i][j] = 0;
				break;
			}
		}
	}
	for (int i = 0; i < W_WIDTH - 2; i += 2)
	{
		for (int j = 0; j < W_HEIGHT - 2; j += 2)
		{
			switch (rand() % 10)
			{
			case 0:
				objects[i][j] = 1;
				objects[i + 1][j] = 1;
				objects[i][j + 1] = 1;
				objects[i + 1][j + 1] =1;
				break;
			}
		}
	}



	//for (int i = 0; i < W_WIDTH; ++i)
	//{
	//	for (int j = 0; j < W_HEIGHT; ++j)
	//	{
	//		char p;

	//		if (objects[i][j]) p = '1';
	//		else p = '0';

	//		if (i == 0 && j == 0) p = '-';
	//		if (i == 88 && j == 88) p = '+';
	//		cout << p;
	//	}
	//	cout << endl;
	//}


	vector<pair<int, int>> road;
	CAstar stars;

	bool val = stars.searchDirections(road, 0, 0, 88, 88);

	if (val == false)
	{
		cout << "fail";
		return 0;
	}

	cout << endl;
	for (int i = 0; i < W_WIDTH; ++i)
	{
		for (int j = 0; j < W_HEIGHT; ++j)
		{
			char p;

			if (objects[i][j]) p = '1';
			else p = '0';
			
			std::vector<pair<int,int>>::iterator it;
			it = find(road.begin(), road.end(), make_pair(i, j));
			if (it != road.end())
				p = '*';
			if (i == 0 && j == 0) p = '-';
			if (i == 88 && j == 88) p = '+';
			cout << p;
		}
		cout << endl;
	}
}