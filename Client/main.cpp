#include "stdafx.h"

using namespace sf;
using namespace std;
#pragma comment (lib, "WS2_32.LIB") 

void CreateWindows();
void DrawWindows();
void InputWindows(Event& e);

void KeyInput(sf::Event& e);

sf::RenderWindow _window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "2D CLIENT");
RenderWindow* window = &_window;

int main() {

	Event event;
	CreateWindows();
	while (window->isOpen()) {
		InputWindows(event);
		DrawWindows();
	}

	return 0;
}

void CreateWindows()
{

}

void DrawWindows()
{
	window->display();
	window->clear(Color::White);
}

void InputWindows(Event& e)
{
	while (window->pollEvent(e)) {

		switch (e.type)
		{
		case Event::Closed:
			window->close();
			break;

		case Event::KeyPressed:
			KeyInput(e);
		}
	}
}

void KeyInput(sf::Event& e)
{
	int direction = -1;
	switch (e.key.code) {
	case sf::Keyboard::Left:
		direction = 2;
		break;
	case sf::Keyboard::Right:
		direction = 3;
		break;
	case sf::Keyboard::Up:
		direction = 0;
		break;
	case sf::Keyboard::Down:
		direction = 1;
		break;
	case sf::Keyboard::Escape:
		window->close();
		break;
	}
	if (-1 != direction) {

	}
}
