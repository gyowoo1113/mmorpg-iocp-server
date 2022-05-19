#include<iostream>
#include<fstream>

int W_WIDTH = 2000;
int W_HEIGHT = 2000;

using namespace std;

int main()
{
	ofstream out("tilemap.txt");
	for (int i = 0; i < W_WIDTH; ++i)
	{
		for (int j = 0; j < W_HEIGHT; ++j)
		{
			out << rand() % 4;
		}
	}
}