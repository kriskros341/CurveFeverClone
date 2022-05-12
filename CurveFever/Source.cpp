#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <iostream>
#include <cmath>
#include <csignal>
#include <map>
#include <thread>
#include <mutex>
#include <chrono>
#include "imgui/imgui.h"
#include "imgui/imgui-SFML.h"


#define PI std::acos(0) * 2
sf::Vector2u screenSize(800, 800);
using std::cout; using std::endl;

bool comp(const char* a, const char* b) {
	int i{};
	if (strlen(a) != strlen(b))
		return false;
	while (a[i] != '\0') {
		if (a[i] != b[i]) {
			return false;
		}
		i++;
	}
	return true;
}

float distance(sf::Vector2f a, sf::Vector2f b) {
	return std::sqrt(std::pow(a.x - b.x, 2) + std::pow(a.y - b.y, 2));
}

sf::Vector2f midpoint(sf::Vector2f v1, sf::Vector2f v2) {
	return (v1 + v2) / 2.0f;
}

sf::Vector2f mmul (float** matrix2, sf::Vector2f v) {
	return {v.x * matrix2[0][0] + v.y * matrix2[0][1], v.x * matrix2[1][0] + v.y * matrix2[1][1]};
}

float** getRotationMatrix(float angle) {
	float sin = std::sin(angle);
	float cos = std::cos(angle);
	float** matrix = new float* [2];
	for (int i{}; i < 2; i++) {
		matrix[i] = new float[2];
	}
	matrix[0][0] = cos;
	matrix[0][1] = -sin;
	matrix[1][0] = sin;
	matrix[1][1] = cos;
	return matrix;
}

float** transpose(float** matrix2) {
	float** n = new float*[2];
	n[0] = new float[2]{ matrix2[0][0], matrix2[1][0] };
	n[1] = new float[2]{ matrix2[0][1], matrix2[1][1] };
	return n;
}

void printv(sf::Vector2f v) {
	std::cout << v.x << " " << v.y << std::endl;
}

// A physics style vector used to mimic velocity and bend angle.
// To be replaced with matrix implementation
class Vector {
	float length{};
	float angle{};
public:
	Vector(float l, float a) : length(l), angle(a) {};
	sf::Vector2f getDisplacement() {
		return { length * std::cos(angle), length * std::sin(angle) };
	}
};

class PositionManager {
	float angle{};
	sf::Vector2f current;
	sf::Vector2f previous;
	sf::Vector2f starting;
	friend class Player;
public:
	PositionManager(sf::Vector2f p) : current(p), starting(p), previous(p) {}
	void restart() {
		current = starting;
		previous = starting;
	}
	float getDistanceFromPrevious() {
		return distance(current, previous);
	}

	float getAngleFromPrevious() {
		return -atan2f(previous.x - current.x, previous.y - current.y) + PI / 2;
	}
	void setAngle(float a) {
		angle = a;
	}
	float getAngle() {
		return angle;
	}
	void changeAngle(float a) {
		angle += a;
	}
	void applyDisplacement(float distance, float angle) {
		current += {distance * std::cos(angle), distance * std::sin(angle)};
	}
	void setPosition(sf::Vector2f& n) {
		previous = current;
		current = n;
	}
	void applyDisplacement(Vector& v) {
		previous = current;
		current = current + v.getDisplacement();
	}
};

class LineManager {
	const int startingLineIndex = 0 - 1; // because it's always incremented by one whenever path is placed;
	int lineIndex = startingLineIndex; 
public:
	std::vector<sf::VertexArray*> linesArray;
	std::vector<std::pair<float, float>> collisionPointMap;
	std::vector<std::pair<float, float>> collisionPointQueue;
	LineManager() {
		linesArray.reserve(100);
		collisionPointMap.reserve(10000);
		collisionPointQueue.reserve(10);
	}
	void initiateLine() {
		lineIndex++;
		sf::VertexArray* currentLine = new sf::VertexArray;
		currentLine->setPrimitiveType(sf::TrianglesStrip);
		linesArray.push_back(currentLine);
	}
	void restart() {
		linesArray.clear();
		collisionPointMap.clear();
		collisionPointQueue.clear();
		lineIndex = startingLineIndex;
		initiateLine();
	}
	int getLineIndex() {
		return lineIndex;
	}
	void setLineindex(int i) {
		lineIndex = i;
	}
};

enum class Inputs {
	Left = 1,
	Right = 2,
	Space = 3,
};


class ControlManager {
public:
	std::string type;
	std::map<Inputs, bool> inputMap;
	/*
		How each player accesses data about their movement.

		Network would create a tcp listener that updates everything based on packets it recieves
		Keyboard would recieve keyboard updates from a seperate thread

	*/
	virtual void poll() = 0;
};

class NetworkControl : public ControlManager {
	sf::TcpSocket& socket;
public:
	NetworkControl(sf::TcpSocket& s) : socket(s) {
		type = "network";
	}
	void poll() {
		
	}
	sf::TcpSocket& getUnderlyingSocket() {
		return socket;
	}
};

//OPTIONAL
class AIControl: public ControlManager {
};
/*
class KeyboardControl: public ControlManager {
	std::map<sf::Keyboard::Key, Inputs> translations;
	std::mutex keyboardMutex;
	bool isPolling = false;
	KeyboardControl() {
		type = "keyboard";
		translations = {
			{sf::Keyboard::D, Inputs::Right},
			{sf::Keyboard::A, Inputs::Left},
			{sf::Keyboard::Space, Inputs::Space},
		};
		isPolling = true;
	}
	void poll(sf::Event& event) {
		switch (event.type) {
		case(sf::Event::KeyPressed): {
			keyboardMutex.lock();
			inputMap[translations[event.key.code]] = true;
			keyboardMutex.unlock();
			break;
		};
		case(sf::Event::KeyReleased): {
			keyboardMutex.lock();
			inputMap[translations[event.key.code]] = false;
			keyboardMutex.unlock();
			break;
		}
		}
	}
};
*/

class Player : public LineManager, public PositionManager {
	bool placesPath = true;
	int size{};
	int id;
public:
	friend class MyRenderWindow;
	Player(sf::Vector2f p = { 400, 400 }, int s = 5) : PositionManager(p), LineManager() {
		size = s;
		placesPath = true;
		initiateLine();
	}
	int getId() {
		return id;
	}

	void moveBy(float distance) {
		if (placesPath) {
			setPath();
		}
		Vector displacement(distance, getAngle());
		applyDisplacement(displacement);
	}

	// Get current position on the map
	sf::Vector2f getPosition() {
		return current;
	}
	// Get starting position
	sf::Vector2f getStarting() {
		return starting;
	}
	// Set whether player places path
	void setPlacesPath(bool v) {
		placesPath = v;
	};
	// Get whether player places path
	bool getPlacesPath() {
		return placesPath;
	};
	// restart game
	void restart() {
		PositionManager::restart();
		LineManager::restart();
		placesPath = true;
	}
	// Get size of player
	int getSize() {
		return size;
	}
	// Set size of player
	void setSize(int n) {
		size = n;
	}

	bool checkForCollision() {
		// check for collision with screen border, then
		// check for collision with path
		if (current.x > screenSize.x ||
			current.x < 0 ||
			current.y > screenSize.y ||
			current.y < 0)
		{
			return true;
		}
		for (auto p = collisionPointMap.begin(); p < collisionPointMap.end(); ++p) {
			if (distance(current, {p->first, p->second}) < size * 2) {
				//collision occured
				return true;
			}
		}
		return false;
	}
	void setPath() {

		// I think it can be done better
		float angleFromPreviousPoint = getAngleFromPrevious();
		Vector c1(size, angleFromPreviousPoint + 1 * PI / 6);
		Vector c2(size, angleFromPreviousPoint - 1 * PI / 6);
		
		// create a rectangle
		sf::Vertex v1 = sf::Vertex(previous + c1.getDisplacement());
		sf::Vertex v2 = sf::Vertex(previous + c2.getDisplacement());
		sf::Vertex v3 = sf::Vertex(current + c1.getDisplacement());
		sf::Vertex v4 = sf::Vertex(current + c2.getDisplacement());
		
		linesArray[getLineIndex()]->append(v3);
		linesArray[getLineIndex()]->append(v4);
		// create point in the center
		sf::Vector2f mid = midpoint(v1.position, v4.position);
		collisionPointQueue.push_back({ mid.x, mid.y });
		// apply new points if out of range of player
		for (auto p = collisionPointQueue.begin(); p != collisionPointQueue.end();) {
			if(distance(current, { p->first, p->second }) > size * 2) {
				collisionPointMap.push_back(*p);
				//rescure p address;
				p = collisionPointQueue.erase(p);
			}
			else {
				//increment p address
				p++;
			}
		}
	}
};

class ControlSignals {
public:
	bool left, right, space;
	ControlSignals(bool l = false, bool r = false, bool s = false) {
		left = l;
		right = r;
		space = s;
	}
};

class NetworkPlayer : public Player {
public:
	sf::TcpSocket*& c;
	ControlSignals ctrl;
	NetworkPlayer(sf::TcpSocket*&co) : Player(), c(co), ctrl() {};
};

class MyRenderWindow : public sf::RenderWindow {
public:
	sf::Clock guiClock;
	MyRenderWindow(sf::VideoMode v, std::string title, sf::ContextSettings c) : sf::RenderWindow(v, title, sf::Style::Close, c) 
	{
		guiClock = sf::Clock();
	}
	void draw(Player& p) {
		sf::CircleShape playerDot;
		playerDot = sf::CircleShape(p.size, 10);
		playerDot.setOrigin(p.size, p.size);
		playerDot.setPosition(p.getPosition());
		sf::RenderWindow::draw(playerDot);
		
		for (auto l = p.linesArray.begin(); l != p.linesArray.end(); ++l) {
			sf::RenderWindow::draw(**l);
		}
	}
	void draw(sf::Shape& s) {
		sf::RenderWindow::draw(s);
	}
	void draw(sf::VertexArray& v) {
		sf::RenderWindow::draw(v);
	}
};

/*
	TODO:
	texture/color of lines
	initially local multiplayer, then game server

	- main menu and a way to measure score
	- a central game server either natively with sfml packet or... let's do js instead of python!
	  - I could try to reverse engineer the packet too and get the best of two worlds
	- https://www.sfml-dev.org/tutorials/2.5/network-socket.php

	Make number of half points dependent on distance traveled
	- n = get distance ##
	- get outer points ##
	- create n points evenly spaced between outer points
	- if it's moving too slow, the distance should add up until it can be achieved

	Create an AI with minimal neural network
	- https://www.youtube.com/watch?v=Ve9TcAkpFgY


*/

// On linked list vs pointer
// https://fylux.github.io/2017/06/29/list_vs_vector/
// Linked lists much worse for small objects
// find better on vector, insert delete on list for front/back

// The queue should be a linked list. (?)



void singleplayer(MyRenderWindow& window) {
	float currentTick{};
	
	// initiate game clock
	sf::Clock clock;
	float elapsed = 0;

	// initiate player and all that has to do with them
	// doing linesArray stuff directly within player object breaks everything ; . ;
	Player player({400, 400}, 2); //starting position, starting size

	//initiate keymap (used to negate keyboard input lag)
	std::map<sf::Keyboard::Key, bool> keymap;
	sf::VertexArray* arrow = new sf::VertexArray;

	//Make it all into a  game object
	auto restart = [&]() {
		player.restart();
	};
	std::vector<std::pair<float, float>>::iterator p;
	auto debug = [&]() {
		arrow->clear();
		sf::Vertex v1(player.getPosition());
		float** g = getRotationMatrix(player.getAngleFromPrevious());
		sf::Vertex v2(player.getPosition() + mmul(g, sf::Vector2f(40, 0)));
		arrow->append(v1);
		arrow->append(v2);
		delete[] g[0];
		delete[] g[1];
		delete[] g;
		sf::CircleShape distanceIndicator(player.getSize() * 2, 5);
		distanceIndicator.setFillColor(sf::Color::Magenta);
		distanceIndicator.setOrigin(player.getSize() * 2, player.getSize() * 2);
		distanceIndicator.setPosition(player.getPosition());
		window.draw(distanceIndicator);
		window.draw(*arrow);
		for (p = player.collisionPointMap.begin(); p < player.collisionPointMap.end(); ++p) {
			sf::CircleShape s(2, 4);
			s.setFillColor(sf::Color::Red);
			s.setPosition(p->first, p->second);
			window.draw(s);
		}
	};

	bool doDebug = false;
	player.setSize(10);
	while (window.isOpen()) {
		sf::Event event;
		while (window.pollEvent(event)) {
			switch (event.type) {
			case(sf::Event::Closed): {
				window.close();
			}
			case(sf::Event::MouseButtonPressed): {
				// pause on button press
				while (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {}
				clock.restart();
				break;
			}
			case(sf::Event::KeyPressed): {
				// update keymap
				keymap[event.key.code] = true;
				// restart the game
				if (sf::Keyboard::isKeyPressed(sf::Keyboard::LControl) && sf::Keyboard::isKeyPressed(sf::Keyboard::R)) {
					player.restart();
				}
				break;
			};
			case(sf::Event::KeyReleased): {
				keymap[event.key.code] = false;
				if(event.key.code == sf::Keyboard::Space) {
					player.initiateLine();
				}
				break;
			}
			}
		}
		elapsed = clock.restart().asMicroseconds();
		// handle keys after they are set for clarity sake
		if (keymap[sf::Keyboard::A]) {
			player.changeAngle(-0.00001 * elapsed);
		}
		if (keymap[sf::Keyboard::D]) {
			player.changeAngle(0.00001 * elapsed);
		}
		doDebug = keymap[sf::Keyboard::B];
		player.setPlacesPath(!keymap[sf::Keyboard::Space]);
		// Move player
		// Do not touch without a commit. Memory safe, but very fragile;
		player.moveBy(0.0003 * elapsed);
		if (player.checkForCollision()) {
			restart();
			continue;
		}
		//draw stuff
		window.clear();
		window.draw(player);
		if(doDebug) debug();
		window.display();
		// increment game tick by a value modified by time between frames
		// this is supposed to make gameplay independent of frames per second and ping in the fututre
		currentTick = currentTick + 5 * elapsed;
	}
	delete arrow;
}

enum class State {
	singleplayer = 1,
	multiplayerMenu,
	menu,
	multiplayer
};


void menu(MyRenderWindow& window, State& s) {
	ImGui::Begin("D");
	if (ImGui::Button("start")) {
		cout << "gg" << endl;
		s = State::singleplayer;
	}
	if (ImGui::Button("multiplayer")) {
		s = State::multiplayerMenu;
	}
	ImGui::Text("Jebac Disa");
	ImGui::End();

	window.clear();
	ImGui::SFML::Render(window);
	window.display();
}
void clearBuff(char* buffer) {
	int i{};
	while (buffer[i] != '\0') {
		buffer[i] = '\0';
		i++;
	}
}

enum class networkFlags {
	JOIN = 1,
	DISCONNECT = 2,
	TEST = 3,
};

class networkClient {
	sf::IpAddress ip;
	char buffer[200]{};
	std::size_t recsize{};
	bool isConnected = false;
	bool isConnecting = false;
	sf::TcpSocket socket;
	const char* w = "welcome";
public:
	std::map<sf::Keyboard::Key, bool> keymap;
	networkClient() {
		ip = sf::IpAddress::getLocalAddress();
	}
	bool getConnected() {
		return isConnected;
	}
	bool getConnecting() {
		return isConnecting;
	}
	bool isWorking() {
		return isConnected || isConnecting;
	}
	void connect() {
		isConnecting = true;
		socket.connect(ip, 53000);
		socket.receive(buffer, sizeof(buffer), recsize);
		cout << buffer << endl;
		if (comp(buffer, w)) {
			std::cout << "CONNECTED!" << endl;
			isConnected = true;
		}
		isConnecting = false;
		clearBuff(buffer);
	};
	void cancelConnect() {
		socket.disconnect();
	}
	void disconnect() {
		socket.disconnect();
		isConnected = false;
	}
	void test() {
		sf::Packet p;
		std::string t = "TEST";
		p << t;
		socket.send(p);
	}
	void join() {
		sf::Packet p;
		std::string t = "JOIN";
		p << t;
		socket.send(p);
	}
	//works on second attempt
	bool start() {
		sf::Packet p;
		std::string t = "START";
		p << t;
		socket.send(p);
		sf::Packet p2;
		socket.receive(p2);
		p2 >> t;
		if (t == "OK") {
			return true;
		}
		else {
			return false;
		}
	}
	sf::TcpSocket& getSocket() {
		return socket;
	}
};

void multiplayer(MyRenderWindow& window, State& s, networkClient& net) {
	Player player;
	std::thread sendLoopThread([&]() {
		while (true) {
			sf::Packet pac1;
			net.getSocket().receive(pac1);
			float newX = 0, newY = 0;
			pac1 >> newX >> newY;
			sf::Vector2f n;
			n.x = newX;
			n.y = newY;
			player.setPosition(n);
		}
		
		});
	std::thread recvLoopThread([&]() {
		while (true) {
			sf::Packet pac2;
			bool right = net.keymap[sf::Keyboard::A], left = net.keymap[sf::Keyboard::D], space = net.keymap[sf::Keyboard::Space];
			std::string procedure = "UPDATE";
			pac2 << procedure << right << left << space;

			std::cout << "UPDATE" << right << left << space << std::endl;
			net.getSocket().send(pac2);
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}
		});
	while (window.isOpen()) {
		sf::Event event;
		while (window.pollEvent(event)) {
			switch (event.type) {
			case(sf::Event::Closed): {
				window.close();
			}
			case(sf::Event::KeyPressed): {
				net.keymap[event.key.code] = true;
				break;
			};
			case(sf::Event::KeyReleased): {
				net.keymap[event.key.code] = false;
				break;
			}
			}
		}
		window.clear();
		window.draw(player);
		window.display();
	}

}
void multiplayerMenu(MyRenderWindow& window, State& s, networkClient& net) {
	if (ImGui::Button("back")) {
		s = State::menu;
		net.disconnect();
	}
	if (ImGui::Button("start")) {
		if (net.start()) {
			s = State::multiplayer;
		};
	}

	if (ImGui::Button("join")) {
		net.join();
	}

	if (ImGui::Button("test")) {
		net.test();
	}

	window.clear();
	ImGui::SFML::Render(window);
	window.display();
}

void multiplayerConnecting(MyRenderWindow& window, State& s, networkClient& net) {
	ImGui::Begin(" ");
	ImGui::Text("Connecting to server");
	if (ImGui::Button("stop")) {
		s = State::menu;
		net.cancelConnect();
	}
	ImGui::End();
	window.clear();
	ImGui::SFML::Render(window);
	window.display();
}
sf::Mutex globalMutex;

class Game {
	bool inProgress = false;
public:
	std::vector<NetworkPlayer*> players;
	void start() {
		for (std::vector<NetworkPlayer*>::iterator p = players.begin(); p != players.end(); ++p) {
			NetworkPlayer* playerInstance = *p;
			sf::Packet pac;
			std::string msg = "OK";
			pac << msg;
			std::cout << msg << std::endl;
			playerInstance->c->send(pac);
		}
		inProgress = true;
		runGameLoop();
		//rungameloop thread
	}
	void runGameLoop() {
		sf::Time elapsed;
		sf::Clock mainClock;
		while (inProgress && players.size() > 0) {
			for (std::vector<NetworkPlayer*>::iterator p = players.begin(); p != players.end(); ++p) {
				NetworkPlayer* PlayerInstance = *p;
				std::cout << PlayerInstance->ctrl.left << PlayerInstance->ctrl.right << std::endl;
				PlayerInstance->changeAngle(PlayerInstance->ctrl.left * -0.00001 * elapsed.asMicroseconds());
				PlayerInstance->changeAngle(PlayerInstance->ctrl.right * 0.00001 * elapsed.asMicroseconds());
				PlayerInstance->moveBy(0.00003 * elapsed.asMicroseconds());
				if (PlayerInstance->checkForCollision()) {
					std::cout << "PLAYER COLLIDED" << std::endl;
				}
			}
			sendUpdates();
			elapsed = mainClock.restart();
		}
	}
	void sendUpdates() {
		for (std::vector<NetworkPlayer*>::iterator p = players.begin(); p != players.end(); ++p) {
			NetworkPlayer*& PlayerInstance = *p;
			sf::Packet pac;
			sf::Vector2f newPosition = PlayerInstance->getPosition();
			pac << newPosition.x << newPosition.y;
			PlayerInstance->c->send(pac);
		}

	}
	void update() {
		
	}
};

class Server {
public:
	sf::TcpListener listener;
	std::size_t recsize{};
	sf::SocketSelector selector;
	std::vector<sf::TcpSocket*> sockets;
	std::vector<Game*> games;
	void parseRecieved(sf::Packet p, sf::TcpSocket*& s) {
		std::string st;
		p >> st;
		std::cout << st << std::endl;
		if (st == "TEST") {
			std::cout << st << std::endl;
			std::cout << games[0]->players.size() << std::endl;
		}
		if (st == "START") {
			std::thread* t = new std::thread([&]() {games[0]->start();});
		}
		if (st == "UPDATE") {
			bool isFound = false;
			NetworkPlayer* savedPlayer = 0;
			for (std::vector<Game*>::iterator g = games.begin(); g != games.end(); ++g) {
				Game*& f = *g; // I think only the last *& matters since these all are pointers, but whatever
				for (std::vector<NetworkPlayer*>::iterator p = f->players.begin(); p != f->players.end(); ++p) {
					NetworkPlayer*& o = *p;
					sf::TcpSocket*& playerSocket = o->c;
					if (playerSocket == s) {
						isFound = true;
						savedPlayer = o;
						break;
					}
				}
				if (isFound) {
					break;
				}
			}
			if (isFound) {
				bool left = false;
				bool right = false;
				bool space = false;
				p >> left >> right >> space;
				std::cout << "UPDATING CONTROLS ON " << s->getRemotePort() << ": " << left << right << space << std::endl;
				savedPlayer->ctrl = ControlSignals(left, right, space);
			}
		}
		if (st == "JOIN") {
			bool isFound = false;
			NetworkPlayer* savedPlayer = 0;
			for (std::vector<Game*>::iterator g = games.begin(); g != games.end(); ++g) {
				Game*& f = *g;
				for (std::vector<NetworkPlayer*>::iterator p = f->players.begin(); p != f->players.end(); ++p) {
					NetworkPlayer*& o = *p;
					sf::TcpSocket*& playerSocket = o->c;
					if (playerSocket == s) {
						isFound = true;
						savedPlayer = o;
						break;
					}
				}
				if (isFound) {
					break;
				}
			}


			if (isFound) {
				std::cout << "PLAYER ALREADY EXISTS FROM " << s->getRemoteAddress() << ":" << s->getRemotePort() << std::endl;
			}
			else {
				NetworkPlayer* playerInstance = new NetworkPlayer(s);
				games[0]->players.push_back(playerInstance);
				sf::Packet ps;
				std::cout << "PLAYER CREATED FROM " << s->getRemoteAddress() << ":" << s->getRemotePort() << std::endl;
				s->send(ps);
			}
		}
	};
	void start() {
		games.push_back(new Game);
		std::thread t([&]() {acceptLoop();});
		std::thread y([&]() {recvLoop();});
		t.join();
		y.join();
	};
	void acceptLoop() {
		while (true) {
			if (listener.listen(53000) != sf::Socket::Done) {
				std::cout << "JD" << std::endl;
			}
			sf::TcpSocket* client = new sf::TcpSocket();

			char w[] = "welcome";
			if (listener.accept(*client) != sf::Socket::Done) {
				std::cout << "CD" << std::endl;
			}
			sockets.push_back(client);
			client->send(w, sizeof(w));
			selector.add(*client);
			std::cout << "client accepted" << endl;
		}
	};
	void recvLoop()
	{
		while (true) {
			if (selector.wait(sf::seconds(10.f))) {
				for (auto a = sockets.begin(); a != sockets.end();) {
					if (selector.isReady(**a)) {
						char buff[200]{};
						sf::Packet p;
						sf::Socket::Status s = (*a)->receive(p);
						if (s == sf::Socket::Status::Disconnected)
						{
							selector.remove(**a);
							//THIS IS SO FUCKING RETARDED
							a = sockets.erase(a);
							//delete *a;
							std::cout << sockets.size() << std::endl;
						}	else {
							parseRecieved(p, *a);
							a++;
						};
					}
					else {
						a++;
					}
					}
				}
			 else {
				std::cout << "timeout" << std::endl;
			}
		}
	};
};

void server() {
/*whenever a user enters there is a huge lag*/
	Server s;
	s.start();
}


void client() {
	sf::ContextSettings settings;
	settings.antialiasingLevel = 0.0;
	MyRenderWindow window(sf::VideoMode(screenSize.x, screenSize.y), "SFML", settings);
	window.setFramerateLimit(60);
	ImGui::SFML::Init(window);
	networkClient net;

	/*
		choice of game mode - single player, multi player
	*/
	State currentState = State::menu;
	while (window.isOpen()) {
		sf::Event event;
		while (window.pollEvent(event)) {
			ImGui::SFML::ProcessEvent(window, event);
			sf::Vector2i mousePosition = sf::Mouse::getPosition(window);
			switch (event.type) {
			case(sf::Event::Closed): {
				window.close();
			}
			}
		}
		ImGui::SFML::Update(window, window.guiClock.restart());
		switch (currentState) {
		case State::menu: {
			menu(window, currentState);
			break;
		};
		case State::singleplayer: {
			singleplayer(window);
			break;
		}
		case State::multiplayer: {
			multiplayer(window, currentState, net);
			break;
		}
		case State::multiplayerMenu: {
			if (!net.isWorking()) {
				net.connect();
			}
			if (net.getConnected()) {
				multiplayerMenu(window, currentState, net);
			}
			else {
				multiplayerConnecting(window, currentState, net);
			}
			break;
		}
		}
	}
	ImGui::SFML::Shutdown();
}

int main(int argc, char* argv[]) {
	// initiate window and globally used values
	bool testServer = false;
	if (argc > 1) {
		if ((int)(char)argv[1][0] == (int)'h') {
			testServer = !testServer;
		}
	}
	if (testServer) {
		server();
	}
	else {
		client();
	}
}
