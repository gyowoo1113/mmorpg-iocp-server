#include<iostream>
#include<fstream>

using namespace std;

int W_WIDTH = 2000;
int W_HEIGHT = 2000;

int objects[2000][2000] = {};

int main()
{
	for (int i = 0; i < W_WIDTH; ++i)
	{
		for (int j = 0; j < W_HEIGHT; ++j)
		{
			switch (rand() % 20)
			{
			case 1:
				objects[i][j] = 5;
				break;
			case 2:
				objects[i][j] = 6;
				break;
			case 5:
				objects[i][j] = 7;
				break;
			case 6:
				objects[i][j] = 8;
				break;
			case 7:
				objects[i][j] = 9;
				break;
			case 8:
				objects[i][j] = 10;
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
				objects[i + 1][j] = 2;
				objects[i][j + 1] = 3;
				objects[i + 1][j + 1] = 4;
				break;
			}
		}
	}


	ofstream out("objects.txt");
	for (int i = 0; i < W_WIDTH; ++i)
	{
		for (int j = 0; j < W_HEIGHT; ++j)
		{
			out << objects[i][j];
		}
	}
}