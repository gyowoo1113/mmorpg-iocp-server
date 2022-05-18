#include "stdafx.h"
#include "Player.h"
#include <deque>
#include <random>
#include "protocol.h"

using namespace sf;
using namespace std;
#pragma comment (lib, "WS2_32.LIB") 

void CreateWindows();
void DrawWindows();
void InputWindows(Event& e);

void KeyInput(sf::Event& e);
Text setTextMessage(string str);
void setMessage();


sf::RenderWindow _window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "2D CLIENT");
RenderWindow* window = &_window;
sf::Text playerText;
sf::Text chattingText;
sf::Font font;

int g_left_x = 0;
int g_top_y = 0;
int g_myid;

string s = "me";
CPlayer player(s, 3, 80, 20);
CPlayer players[MAX_USER];
sf::TcpSocket socket;

string userChatting;
deque<Text> chat;
bool isChatting = false;

void ProcessPacket(char* ptr)
{
	static bool first_time = true;
	switch (ptr[1])
	{
	case SC_LOGIN_INFO:
	{
		SC_LOGIN_INFO_PACKET* packet = reinterpret_cast<SC_LOGIN_INFO_PACKET*>(ptr);
		g_myid = packet->id;
		player.move(packet->x, packet->y);
		g_left_x = packet->x - 8;
		g_top_y = packet->y - 8;
		player.setActive(true);
		break;
	}

	case SC_ADD_PLAYER:
	{
		SC_ADD_PLAYER_PACKET* my_packet = reinterpret_cast<SC_ADD_PLAYER_PACKET*>(ptr);
		int id = my_packet->id;

		if (id < MAX_USER) {
			players[id].move(my_packet->x, my_packet->y);
			players[id].setActive(true);
		}
		break;
	}

	case SC_MOVE_PLAYER:
	{
		SC_MOVE_PLAYER_PACKET* my_packet = reinterpret_cast<SC_MOVE_PLAYER_PACKET*>(ptr);
		int other_id = my_packet->id;
		if (other_id == g_myid) {
			g_left_x = my_packet->x - 8;
			g_top_y = my_packet->y - 8;
			player.move(my_packet->x, my_packet->y);
		}
		else if (other_id < MAX_USER) {
			players[other_id].move(my_packet->x, my_packet->y);
		}
		break;
	}

	case SC_REMOVE_PLAYER:
	{
		SC_REMOVE_PLAYER_PACKET* my_packet = reinterpret_cast<SC_REMOVE_PLAYER_PACKET*>(ptr);
		int other_id = my_packet->id;
		if (other_id == g_myid) {
			player.setActive(false);
		}
		else if (other_id < MAX_USER) {
			players[other_id].setActive(false);
		}
		break;
	}
	default:
		printf("Unknown PACKET type [%d]\n", ptr[1]);
	}
}

void send_packet(void* packet)
{
	unsigned char* p = reinterpret_cast<unsigned char*>(packet);
	size_t sent = 0;
	socket.send(packet, p[0], sent);
}

void process_data(char* net_buf, size_t io_byte)
{
	char* ptr = net_buf;
	static size_t in_packet_size = 0;
	static size_t saved_packet_size = 0;
	static char packet_buffer[BUF_SIZE];

	while (0 != io_byte) {
		if (0 == in_packet_size) in_packet_size = ptr[0];
		if (io_byte + saved_packet_size >= in_packet_size) {
			memcpy(packet_buffer + saved_packet_size, ptr, in_packet_size - saved_packet_size);
			ProcessPacket(packet_buffer);
			ptr += in_packet_size - saved_packet_size;
			io_byte -= in_packet_size - saved_packet_size;
			in_packet_size = 0;
			saved_packet_size = 0;
		}
		else {
			memcpy(packet_buffer + saved_packet_size, ptr, io_byte);
			saved_packet_size += io_byte;
			io_byte = 0;
		}
	}
}


int backgrounds[W_WIDTH][W_HEIGHT] = {};

void makeMap()
{
	for (int i = 0; i < W_WIDTH; ++i)
	{
		for (int j = 0; j < W_HEIGHT; ++j)
		{
			backgrounds[i][j] = rand() % 4;
		}
	}
}

sf::Texture* maptiles;
vector<CGameObject> maptile;
sf::RectangleShape shape;
sf::RectangleShape shape2;

void client_initialize()
{
	maptiles = new sf::Texture;
	// 총 14개 타일종류
	maptiles->loadFromFile("../Resource/background.png");

	for (int i = 0; i < 4; ++i)
	{
		CGameObject maketile = CGameObject{ *maptiles, 16*i, 0, 16, 16 };
		maketile.setSpriteScale(4.0f, 4.0f);
		maptile.emplace_back(maketile);
	}

	shape.setSize(Vector2f(WINDOW_WIDTH, CHAT_SIZE * 5));
	shape.setPosition(Vector2f(0, WINDOW_HEIGHT - CHAT_SIZE * 6));
	shape.setFillColor(Color(0, 0, 0, 125));

	shape2.setSize(Vector2f(WINDOW_WIDTH, CHAT_SIZE));
	shape2.setPosition(Vector2f(0, WINDOW_HEIGHT - CHAT_SIZE));
	shape2.setFillColor(Color(0, 0, 0, 200));
}


int main() {
	sf::Socket::Status status = socket.connect("127.0.0.1", PORT_NUM);
	socket.setBlocking(false);

	CS_LOGIN_PACKET p;
	p.size = sizeof(CS_LOGIN_PACKET);
	p.type = CS_LOGIN;
	strcpy_s(p.name, "TEMP");
	send_packet(&p);

	if (status != sf::Socket::Done) {
		wcout << L"서버와 연결할 수 없습니다.\n";
		while (true);
	}

	makeMap();
	client_initialize();

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
	chattingText.setPosition(Vector2f(0, WINDOW_HEIGHT - CHAT_SIZE - 5));
}

void DrawWindows()
{
	char net_buf[BUF_SIZE];
	size_t	received;

	auto recv_result = socket.receive(net_buf, BUF_SIZE, received);
	if (recv_result == sf::Socket::Error)
	{
		wcout << L"Recv 에러!";
		while (true);
	}
	if (recv_result != sf::Socket::NotReady)
		if (received > 0) process_data(net_buf, received);

	player.setParameter(playerText);

	for (int i = 0; i < SCREEN_WIDTH; ++i)
	{
		for (int j = 0; j < SCREEN_HEIGHT; ++j)
		{
			int tile_x = i + g_left_x;
			int tile_y = j + g_top_y;
			if ((tile_x < 0) || (tile_y < 0)) continue;
			int index = backgrounds[tile_x][tile_y];
			maptile[index].spriteMove(64 * i, 64 * j);
			maptile[index].spriteDraw();
		}
	}

	if (isChatting)
	{
		window->draw(shape);
		//window->draw(shape2);

		deque<Text>::reverse_iterator itor;
		int cnt = 0;
		int chat_start_h = WINDOW_HEIGHT - CHAT_SIZE * 2 - 5;
		for (itor = chat.rbegin(); itor != chat.rend(); ++itor)
		{
			itor->setPosition(Vector2f(0, chat_start_h - cnt * CHAT_SIZE));
			window->draw(*itor);
			++cnt;
		}


		chattingText.setString(userChatting);
		window->draw(chattingText);
	}

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
			if (!isChatting) return;

			userChatting += ' ';
			break;

		case sf::Keyboard::BackSpace:
			if (!isChatting) return;

			if (userChatting.size() > 0)
				userChatting.pop_back();
			break;

		case Keyboard::Return:
			if (userChatting.size() > 0 && isChatting)
				setMessage();
			else if (userChatting.size() == 0)
			{
				isChatting = !isChatting;
			}
			break;

	}

	if (-1 != direction) {
		CS_MOVE_PACKET p;
		p.size = sizeof(p);
		p.type = CS_MOVE;
		p.direction = direction;
		send_packet(&p);
	}
}

Text setTextMessage(string str)
{
	sf::Text text;
	text.setFont(font);
	text.setCharacterSize(CHAT_SIZE);
	text.setString(str);

	return text;
}

void setMessage()
{
	string chatting = player.getId() + ":" + userChatting;
	if (chat.size() > 4)
		chat.pop_front();

	chat.push_back(setTextMessage(chatting));

	userChatting.clear();
}