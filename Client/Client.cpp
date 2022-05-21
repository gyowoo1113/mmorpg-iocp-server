#include "stdafx.h"
#include "Player.h"
#include "Monster.h"
#include <deque>
#include <random>
#include <fstream>
#include "protocol.h"

using namespace sf;
using namespace std;
#pragma comment (lib, "WS2_32.LIB") 

void CreateWindows();
void setConnectServer();
void DrawWindows();
void receiveData();
void drawMaps();
void drawChatting();
void InputWindows(Event& e);

void KeyInput(sf::Event& e);
Text setTextMessage(string str);
void setMessage();

void process_data(char* net_buf, size_t io_byte);
void send_packet(void* packet);
void ProcessPacket(char* ptr);

sf::RenderWindow _window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "2D CLIENT");
RenderWindow* window = &_window;
sf::Text playerText;
sf::Text chattingText;
sf::Font font;

int g_left_x = 0;
int g_top_y = 0;
int g_myid;
int direction = -1;

string s = "me";
CPlayer player;
CPlayer players[MAX_USER];
CMonster npcs[NUM_NPC];
sf::TcpSocket socket;

string userChatting;
deque<Text> chat;
bool isChatting = false;
bool isMoving = false;

int backgrounds[W_WIDTH][W_HEIGHT] = {};
int objects[W_WIDTH][W_HEIGHT] = {};

void makeMap()
{
	ifstream in("../Resource/tilemap.txt");
	for (int i = 0; i < W_WIDTH; ++i)
	{
		for (int j = 0; j < W_HEIGHT; ++j)
		{
			char num;
			in >> num;
			backgrounds[i][j] = num -'0';
		}
	}
}

void makeObjects()
{
	ifstream in("../Resource/objects.txt");
	for (int i = 0; i < W_WIDTH; ++i)
	{
		for (int j = 0; j < W_HEIGHT; ++j)
		{
			char num;
			in >> num;
			objects[i][j] = num - 'a';
		}
	}
}

sf::Texture* maptiles;
sf::Texture* playertiles;
sf::Texture* monstertiles;
sf::Texture* objecttiles;
vector<CGameObject> maptile;
vector<CGameObject> objecttile;
sf::RectangleShape shape;
sf::RectangleShape shape2;

float fscale = TILE_WIDTH / 16.0f;

void client_initialize()
{
	maptiles = new sf::Texture;
	playertiles = new Texture;
	objecttiles = new Texture;
	monstertiles = new Texture;
	// 총 14개 타일종류
	maptiles->loadFromFile("../Resource/background.png");
	playertiles->loadFromFile("../Resource/Idle.png");
	objecttiles->loadFromFile("../Resource/objects.png");	
	monstertiles->loadFromFile("../Resource/Flam.png");

	for (int i = 0; i < 14; ++i)
	{
		CGameObject maketile = CGameObject{ *maptiles, 16*i, 0, 16, 16 };
		maketile.setSpriteScale(fscale, fscale);
		maptile.emplace_back(maketile);
	}

	for (int i = 0; i < 10; ++i)
	{
		CGameObject mtile = CGameObject{ *objecttiles, 16 * i, 0, 16, 16 };
		mtile.setSpriteScale(fscale, fscale);
		objecttile.emplace_back(mtile);
	}

	for (auto& pl : npcs) {
		pl = CMonster( *monstertiles, 0, 0, 16, 16,"flam",1,50,10 );
		pl.setSpriteScale(fscale, fscale);
	}

	player = CPlayer(*playertiles, 0, 0, 16, 16, s, 3, 80, 20);
	player.setFrameCount(4);
	player.setSpriteScale(fscale, fscale);

	shape.setSize(Vector2f(WINDOW_WIDTH, CHAT_SIZE * 5));
	shape.setPosition(Vector2f(0, WINDOW_HEIGHT - CHAT_SIZE * 6));
	shape.setFillColor(Color(0, 0, 0, 125));

	shape2.setSize(Vector2f(WINDOW_WIDTH, EXP_HEIGHT));
	shape2.setPosition(Vector2f(0, WINDOW_HEIGHT - EXP_HEIGHT));
	shape2.setFillColor(Color(0, 200, 0));
}


int main() {
	makeMap();
	makeObjects();
	setConnectServer();
	client_initialize();
	CreateWindows();

	window->setFramerateLimit(60);

	sf::Clock clock;

	Event event;
	while (window->isOpen()) {
		sf::Time elapsed = clock.getElapsedTime();
		if (elapsed.asSeconds() > 0.1f) clock.restart();
		else continue;
		
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
	playerText.setOutlineThickness(3.0f);
	chattingText.setFont(font);
	chattingText.setCharacterSize(CHAT_SIZE);
	chattingText.setPosition(Vector2f(0, WINDOW_HEIGHT - CHAT_SIZE - 5 - EXP_HEIGHT));
}

void DrawWindows()
{
	receiveData();

	player.getExp(1);
	player.setParameter(playerText);

	drawMaps();
	drawChatting();

	if (isMoving)
	{
		if (direction != -1)
			player.setState(direction);
		player.animDraw();
	}
	else
	{
		player.initIndex();
		player.draw();
	}

	player.drawAttack();
	for (auto& pl : npcs) pl.draw();

	shape2.setSize(Vector2f(player.getExpRatio()* WINDOW_WIDTH, EXP_HEIGHT));
	window->draw(shape2);

	window->draw(playerText);
	window->display();
	window->clear(Color::Black);
}

void drawMaps()
{
	for (int i = 0; i < SCREEN_WIDTH; ++i)
	{
		for (int j = 0; j < SCREEN_HEIGHT; ++j)
		{
			int tile_x = i + ::g_left_x;
			int tile_y = j + ::g_top_y;
			if ((tile_x < 0) || (tile_y < 0)) continue;
			if ((tile_x >= W_WIDTH) || (tile_y >= W_HEIGHT)) continue;
			int index = backgrounds[tile_x][tile_y];
			maptile[index].spriteMove(16* fscale* i, 16 * fscale * j);
			maptile[index].spriteDraw();
		}
	}

	for (int i = 0; i < SCREEN_WIDTH; ++i)
	{
		for (int j = 0; j < SCREEN_HEIGHT; ++j)
		{
			int tile_x = i + ::g_left_x;
			int tile_y = j + ::g_top_y;
			if ((tile_x < 0) || (tile_y < 0)) continue;
			if ((tile_x >= W_WIDTH) || (tile_y >= W_HEIGHT)) continue;

			int index = objects[tile_x][tile_y] - 1;
			if (index == -1) continue;

			objecttile[index].spriteMove(16 * fscale * i, 16 * fscale * j);
			objecttile[index].spriteDraw();
		}
	}
}

void drawChatting()
{
	if (isChatting == false) return;

	window->draw(shape);

	deque<Text>::reverse_iterator itor;
	int cnt = 0;
	int chat_start_h = WINDOW_HEIGHT - CHAT_SIZE * 2 - 5;
	for (itor = chat.rbegin(); itor != chat.rend(); ++itor)
	{
		float pos = chat_start_h - cnt * CHAT_SIZE;
		itor->setPosition(Vector2f(0,pos));
		window->draw(*itor);
		++cnt;
	}

	chattingText.setString(userChatting);
	window->draw(chattingText);
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

			case Event::KeyReleased:
				isMoving = false;
				player.initIndex();
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
	direction = -1;
	switch (e.key.code) {
		case sf::Keyboard::Left:
			direction = 2;
			isMoving = true;
			break;
		case sf::Keyboard::Right:
			direction = 3;
			isMoving = true;

			break;
		case sf::Keyboard::Up:
			direction = 1;
			isMoving = true;

			break;
		case sf::Keyboard::Down:
			direction = 0;
			isMoving = true;

			break;
		case sf::Keyboard::Escape:
			window->close();
			break;

		case sf::Keyboard::R:
			player.setAttack();
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


//*** Server *** //
////////////////////////////////////////////

void setConnectServer()
{
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
}

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
		::g_left_x = packet->x - SCREEN_WIDTH/2;
		::g_top_y = packet->y - SCREEN_HEIGHT/2;
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
		else {
			npcs[id - MAX_USER].move(my_packet->x, my_packet->y);
			npcs[id - MAX_USER].setActive(true);
		}
		break;
	}

	case SC_MOVE_PLAYER:
	{
		SC_MOVE_PLAYER_PACKET* my_packet = reinterpret_cast<SC_MOVE_PLAYER_PACKET*>(ptr);
		int other_id = my_packet->id;
		if (other_id == g_myid) {
			::g_left_x = my_packet->x - SCREEN_WIDTH / 2;
			::g_top_y = my_packet->y - SCREEN_HEIGHT / 2;
			player.move(my_packet->x, my_packet->y);
		}
		else if (other_id < MAX_USER) {
			players[other_id].move(my_packet->x, my_packet->y);
		}
		else {
			npcs[other_id - MAX_USER].move(my_packet->x, my_packet->y);
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
		else {
			npcs[other_id - MAX_USER].setActive(false);
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

void receiveData()
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
}


////////////////////////////////////////////////