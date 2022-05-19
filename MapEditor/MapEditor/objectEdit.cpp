#include<iostream>
#include<fstream>

using namespace std;

int W_WIDTH = 2000;
int W_HEIGHT = 2000;

char objects[2000][2000] = {};

int main()
{
	for (int i = 0; i < W_WIDTH; ++i)
	{
		for (int j = 0; j < W_HEIGHT; ++j)
		{
			switch (rand() % 20)
			{
			case 1:
				objects[i][j] = 'f';
				break;
			case 2:
				objects[i][j] = 'g';
				break;
			case 5:
				objects[i][j] = 'h';
				break;
			case 6:
				objects[i][j] = 'i';
				break;
			case 7:
				objects[i][j] = 'j';
				break;
			case 8:
				objects[i][j] = 'k';
				break;

			default:
				objects[i][j] = 'a';
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
				objects[i][j] = 'b';
				objects[i + 1][j] = 'c';
				objects[i][j + 1] = 'd';
				objects[i + 1][j + 1] = 'e';
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