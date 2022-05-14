#include "stdafx.h"
#include "Player.h"

using namespace sf;
using namespace std;
#pragma comment (lib, "WS2_32.LIB") 

void CreateWindows();
void DrawWindows();
void InputWindows(Event& e);

void KeyInput(sf::Event& e);

sf::RenderWindow _window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "2D CLIENT");
RenderWindow* window = &_window;
sf::Text playerText;
sf::Font font;

string s = "me";
CPlayer player(s, 3, 80, 20);

int main() {

	CreateWindows();

	Event event;
	while (window->isOpen()) {
		InputWindows(event);
		DrawWindows();
	}

	return 0;
}

void CreateWindows()
{
	if (!font.loadFromFile("../Resource/Font/Goyang.ttf"))
	{
		cout << "fail!!" << endl;
	}

	playerText.setFont(font);
	playerText.setCharacterSize(24);
}

void DrawWindows()
{
	player.setParameter(playerText);
	window->draw(playerText);
	window->display();
	window->clear(Color::Black);
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
