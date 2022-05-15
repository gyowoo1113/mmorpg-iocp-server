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
sf::Text chattingText;
sf::Font font;

string s = "me";
CPlayer player(s, 3, 80, 20);
string userChatting;

int main() {

	CreateWindows();
	window->setFramerateLimit(60);

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
	playerText.setPosition(Vector2f(0, 0));
	chattingText.setFont(font);
	chattingText.setCharacterSize(CHAT_SIZE);
	chattingText.setPosition(Vector2f(0, WINDOW_HEIGHT - CHAT_SIZE));
}

void DrawWindows()
{
	player.getExp(1);
	player.setParameter(playerText);
	chattingText.setString(userChatting);

	window->draw(chattingText);
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
				break;

			case Event::TextEntered:
				if (!isChatting) return;

				if (32 < e.text.unicode && e.text.unicode < 128)
					userChatting += (char)e.text.unicode;
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
		case sf::Keyboard::Space:

			userChatting += ' ';
			break;

		case sf::Keyboard::BackSpace:

			if (userChatting.size() > 0)
				userChatting.pop_back();
			break;
	}
	if (-1 != direction) {

	}
}
