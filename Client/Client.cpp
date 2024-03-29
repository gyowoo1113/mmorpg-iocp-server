﻿#include "stdafx.h"
#include "Player.h"
#include "Monster.h"
#include <deque>
#include <random>
#include <fstream>
#include "protocol.h"

using namespace sf;
using namespace std;
#pragma comment (lib, "WS2_32.LIB") 

void sendMessage();
void CreateWindows();
void setConnectServer();
void DrawWindows();
void drawUI();
void drawPlayers();
void receiveData();
void drawMaps();
void drawChatting();
void InputWindows(Event& e);

void KeyInput(sf::Event& e);
Text setTextMessage(string str,bool isSystemMessage = false);
void setMessage(char* text, bool isSystem = false);

void process_data(char* net_buf, size_t io_byte);
void send_packet(void* packet);
void ProcessPacket(char* ptr);

sf::RenderWindow _window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "2D CLIENT");
RenderWindow* window = &_window;
sf::Text playerText;
sf::Text chattingText;
sf::Font g_font;

int g_left_x = 0;
int g_top_y = 0;
int g_myid;
int direction = -1;

CPlayer player;
unordered_map<int, CPlayer> players;
unordered_map<int, CMonster> npcs;

sf::TcpSocket socket;

string playerName;
string userChatting;
deque<Text> chat;
bool isChatting = false;
bool isShowChatting = true;
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
sf::Texture* objecttiles;
sf::Texture* numbertiles;

vector<Texture*> effectTiles;
vector<Texture*> monstertiles;
vector<CGameObject> maptile;
vector<CGameObject> objecttile;
vector<CGameObject> skilltile;
sf::RectangleShape shape;
sf::RectangleShape shape2;

vector<sf::RectangleShape> skill_shapes;

float fscale = TILE_WIDTH / 16.0f;

void client_initialize()
{
	maptiles = new sf::Texture;
	playertiles = new Texture;
	objecttiles = new Texture;
	numbertiles = new Texture;
	// 총 14개 타일종류
	maptiles->loadFromFile("../Resource/background.png");
	playertiles->loadFromFile("../Resource/Idle.png");
	objecttiles->loadFromFile("../Resource/objects.png");
	numbertiles->loadFromFile("../Resource/num.png");

	string res_effect_name[4] = { "slash","Fire","Circle","Shield"};

	for (int i = 0; i < 4; ++i) {
		Texture* eff  = new Texture;
		string m_name = "../Resource/" + res_effect_name[i] + ".png";
		eff->loadFromFile(m_name);
		effectTiles.emplace_back(eff);
	}

	string m_res_name[4] = { "mushroom","Flam","Flam2","Skull" };

	for (int i = 0; i < 4; ++i) {
		Texture* mons = new Texture;
		string m_name = "../Resource/" + m_res_name[i] + ".png";
		mons->loadFromFile(m_name);
		monstertiles.emplace_back(mons);
	}

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

	for (int i = 0; i < 3; ++i)
	{
		CGameObject stile = CGameObject{ *numbertiles, 16 * i, 0, 16, 16 };
		stile.setSpriteScale(fscale, fscale);
		skilltile.emplace_back(stile);
	}

	player = CPlayer(*playertiles,*effectTiles[0], 0, 0, 16, 16, playerName, 3, 80, 20);
	player.setFrameCount(4);
	player.setSpriteScale(fscale, fscale);

	shape.setSize(Vector2f(WINDOW_WIDTH, CHAT_SIZE * 5));
	shape.setPosition(Vector2f(0, WINDOW_HEIGHT - CHAT_SIZE * 6));
	shape.setFillColor(Color(0, 0, 0, 125));

	shape2.setSize(Vector2f(WINDOW_WIDTH, EXP_HEIGHT));
	shape2.setPosition(Vector2f(0, WINDOW_HEIGHT - EXP_HEIGHT));
	shape2.setFillColor(Color(0, 200, 0));

	for (int i = 0; i < 3; ++i) {
		sf::RectangleShape temp_shape;
		temp_shape.setSize(Vector2f(32, 32));
		temp_shape.setPosition(Vector2f(32*i, 32));
		temp_shape.setFillColor(Color(0, 0, 0, 120));
		skill_shapes.push_back(temp_shape);
	}
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
	if (!g_font.loadFromFile("../Resource/Font/Goyang.ttf"))
	{
		cout << "fail!!" << endl;
	}

	playerText.setFont(g_font);
	playerText.setCharacterSize(24);
	playerText.setPosition(Vector2f(0, 0));
	playerText.setOutlineThickness(3.0f);
	chattingText.setFont(g_font);
	chattingText.setCharacterSize(CHAT_SIZE);
	chattingText.setPosition(Vector2f(0, WINDOW_HEIGHT - CHAT_SIZE - 5 - EXP_HEIGHT));
}

void DrawWindows()
{
	receiveData();

	drawMaps();
	drawChatting();
	drawPlayers();
	drawUI();

	window->display();
	window->clear(Color::Black);
}

void drawUI()
{
	player.setParameter(playerText);
	shape2.setSize(Vector2f(player.getExpRatio() * WINDOW_WIDTH, EXP_HEIGHT));
	window->draw(shape2);
	window->draw(playerText);

	for (int i=0; i<3; ++i)
	{
		skilltile[i].spriteMove(16 * fscale * i, 16 * fscale*1);
		skilltile[i].spriteDraw();

		if(player.isCoolDown(i) == false)
			window->draw(skill_shapes[i]);
	}
}

void drawPlayers()
{
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

	for (auto& pl : players) pl.second.draw();
	for (auto& pl : npcs) pl.second.animDraw();
	for (auto& pl : players) pl.second.drawAttacks();
	for (auto& pl : npcs) pl.second.drawAttack();

	player.drawAttacks();
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
	if (isChatting)
		window->draw(shape);

	deque<Text>::reverse_iterator itor;
	int cnt = 0;
	int chat_start_h = WINDOW_HEIGHT - CHAT_SIZE * 2 - 10;
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

		case sf::Keyboard::Num1: {
			if (isChatting) return;
			if (!player.isCoolDown(0)) return;

			player.setAttack(0);

			CS_ATTACK_PACKET p;
			p.size = sizeof(p);
			p.type = CS_ATTACK;
			p.skill_type = 0;
			send_packet(&p);

			break;
		}
		case sf::Keyboard::Num2: {
			if (isChatting) return;
			if (!player.isCoolDown(1)) return;
			player.setAttack(1);

			CS_ATTACK_PACKET p;
			p.size = sizeof(p);
			p.type = CS_ATTACK;
			p.skill_type = 1;
			send_packet(&p);

			break;
		}
		case sf::Keyboard::Num3: {
			if (isChatting) return;
			if (!player.isCoolDown(2)) return;

			player.setAttack(2);

			CS_ATTACK_PACKET p;
			p.size = sizeof(p);
			p.type = CS_ATTACK;
			p.skill_type = 2;
			send_packet(&p);

			break;
		}
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
				sendMessage();
			else if (userChatting.size() == 0)
			{
				isChatting = !isChatting;
			}
			break;

		case Keyboard::Num4:
			isShowChatting = !isShowChatting;
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

Text setTextMessage(string str, bool isSystemMessage)
{
	sf::Text text;
	text.setFont(g_font);
	text.setCharacterSize(CHAT_SIZE);
	text.setString(str);

	if (isSystemMessage)
	{
		text.setFillColor(Color(0, 0 ,180));
		//text.setOutlineThickness(0.4f);
		//text.setOutlineColor(Color::Blue);
	}

	return text;
}

void setMessage(char* text,bool isSystem)
{
	if (chat.size() > CHAT_LINE)
		chat.pop_front();

	chat.push_back(setTextMessage(text, isSystem));
}

void sendMessage()
{
	string chatting = player.getName() + ":" + userChatting;

	CS_CHAT_PACKET p;
	p.size = sizeof(SC_CHAT_PACKET) - sizeof(p.mess) + chatting.size() + 1;
	p.type = CS_CHAT;
	strcpy_s(p.mess, chatting.c_str());
	send_packet(&p);

	userChatting.clear();
}

//*** Server *** //
////////////////////////////////////////////

void setConnectServer()
{
	cout <<"IP : ";
	string ip;
	cin >> ip;

	sf::Socket::Status status = socket.connect(ip.c_str(), PORT_NUM);
	socket.setBlocking(false);
	
	cout << "ID: ";
	char id[20];
	scanf_s("%s", id, sizeof(id));
	playerName.replace(playerName.begin(), playerName.end(), id);


	if (status != sf::Socket::Done) {
		wcout << L"서버와 연결할 수 없습니다.\n";
		while (true);
	}

	CS_LOGIN_PACKET p;
	p.size = sizeof(CS_LOGIN_PACKET);
	p.type = CS_LOGIN;
	strcpy_s(p.name, id);
	send_packet(&p);
}

void ProcessPacket(char* ptr)
{
	static bool first_time = true;
	switch (ptr[1])
	{
	case SC_LOGIN_FAIL:
	{
		cout << "login failed !!!\n";
		exit(0);
		break;
	}
	case SC_LOGIN_OK:
	{
		SC_LOGIN_OK_PACKET* packet = reinterpret_cast<SC_LOGIN_OK_PACKET*>(ptr);
		g_myid = packet->id;
		player.move(packet->x, packet->y);
		::g_left_x = packet->x - SCREEN_WIDTH/2;
		::g_top_y = packet->y - SCREEN_HEIGHT/2;
		player.setStatus(packet->hp, packet->level, packet->exp);
		player.calculateMaxExp();
		player.setActive(true);
		break;
	}

	case SC_ADD_OBJECT:
	{
		SC_ADD_OBJECT_PACKET* my_packet = reinterpret_cast<SC_ADD_OBJECT_PACKET*>(ptr);
		int id = my_packet->id;

		if (id < MAX_USER) {
			if (0 != players.count(id)) break;


			// level, exp , hp 정보 불필요
			players[id] = CPlayer(*playertiles, *effectTiles[0], 0, 0, 16, 16, my_packet->name, 0,0,0);
			players[id].setSpriteScale(fscale, fscale);
			players[id].move(my_packet->x, my_packet->y);
			players[id].setActive(true);
		}
		else {
			if (0 != npcs.count(id)) break;

			npcs[id] = CMonster(*monstertiles[my_packet->race],*effectTiles[1], 0, 0, 16, 16, my_packet->name, my_packet->level, my_packet->hp, 0);
			npcs[id].setSpriteScale(fscale, fscale);
			npcs[id].move(my_packet->x, my_packet->y);
			npcs[id].setActive(true);
		}
		break;
	}

	case SC_MOVE_OBJECT:
	{
		SC_MOVE_OBJECT_PACKET* my_packet = reinterpret_cast<SC_MOVE_OBJECT_PACKET*>(ptr);
		int other_id = my_packet->id;
		if (other_id == g_myid) {
			::g_left_x = my_packet->x - SCREEN_WIDTH / 2;
			::g_top_y = my_packet->y - SCREEN_HEIGHT / 2;
			player.move(my_packet->x, my_packet->y);
		}
		else if (other_id < MAX_USER) {
			if (players.count(other_id) == 0) break;
			players[other_id].move(my_packet->x, my_packet->y);
		}
		else {
			if (npcs.count(other_id) == 0) break;
			npcs[other_id].move(my_packet->x, my_packet->y);
		}
		break;
	}

	case SC_REMOVE_OBJECT:
	{
		SC_REMOVE_OBJECT_PACKET* my_packet = reinterpret_cast<SC_REMOVE_OBJECT_PACKET*>(ptr);
		int other_id = my_packet->id;
		if (other_id == g_myid) {
			player.setActive(false);
		}
		else if (other_id < MAX_USER) {
			if (players.count(other_id) == 0) break;
			players[other_id].setActive(false);
			players.erase(other_id);
		}
		else {
			if (npcs.count(other_id) == 0) break;
			npcs[other_id].setActive(false);
			npcs.erase(other_id);
		}
		break;
	}
	case SC_CHAT:
	{
		SC_CHAT_PACKET* my_packet = reinterpret_cast<SC_CHAT_PACKET*>(ptr);
		int other_id = my_packet->id;

		bool isSystem = (other_id == -1) ? true : false;
		setMessage(my_packet->mess,isSystem);
		break;
	}
	case SC_STAT_CHANGE:
	{	
		SC_STAT_CHANGE_PACKET* packet = reinterpret_cast<SC_STAT_CHANGE_PACKET*>(ptr);
		int c_id = packet->id;

		if (c_id == g_myid) {
			player.setStatus(packet->hp, packet->level, packet->exp);
		}
		else if (c_id < MAX_USER){
			if (players.count(c_id) == 0) break;
			players[c_id].setStatus(packet->hp, packet->level, packet->exp);
		}
		else {
			if (npcs.count(c_id) == 0) break;
			npcs[c_id].setHp(packet->hp);
		}

		break; 
	}
	case SC_ATTACK:
	{
		SC_ATTACK_PACKET* packet = reinterpret_cast<SC_ATTACK_PACKET*>(ptr);
		int c_id = packet->id;
		
		if (c_id == g_myid) {

			if (packet->active_type == 1)
				player.activeCoolDown(packet->skill_type);
			if (packet->active_type == 2)
				player.releaseAttack(packet->skill_type);
		}
		else if (c_id < MAX_USER) {
			if (players.count(c_id) == 0) break;

			if (packet->active_type == 0)
				players[c_id].setAttack(packet->skill_type);
			else if (packet->active_type == 2)
				players[c_id].releaseAttack(packet->skill_type);
		}
		else {
			if (npcs.count(c_id) == 0) break;
			npcs[c_id].effectMove(packet->x, packet->y);
			npcs[c_id].setAttack();
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
	static size_t nPacket = 0;
	static size_t nSavdPacket = 0;
	static char packetBuffer[BUF_SIZE];

	while (0 != io_byte) {
		if (nPacket == 0) {
			REBUILD_PACKET* packet = reinterpret_cast<REBUILD_PACKET*>(ptr);
			nPacket = static_cast<size_t>(packet->size);
		}

		size_t nRemainPacket = nPacket - nSavdPacket;
		if (io_byte < nRemainPacket) break;

		memcpy(packetBuffer + nSavdPacket, ptr, nRemainPacket);
		ProcessPacket(packetBuffer);
		ptr += nRemainPacket;
		io_byte -= nRemainPacket;
		nPacket = nSavdPacket = 0;
	}

	memcpy(packetBuffer + nSavdPacket, ptr, io_byte);
	nSavdPacket += io_byte;
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