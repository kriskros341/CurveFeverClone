#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>
#include <vector>
#include <sstream>
#include "imgui/imgui.h"
#include "imgui/imgui-SFML.h"
#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>


#define PI std::acos(0) * 2
const sf::Vector2u screenSize(1000, 1000);
using std::cout; using std::endl;

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
	std::mutex positionMutex;
	PositionManager(sf::Vector2f p) : current(p), starting(p), previous(p) {}
	void restart() {
		current = starting;
		previous = starting;
	}
	PositionManager(float radius = 300) {
		angle = rand() % 360;
		cout << std::cos(angle) << ", " << std::sin(angle) << ", radians: " << angle * PI / 180 << endl;

		sf::Vector2f r = { radius, 0 };
		sf::Vector2f secr = {screenSize.x/2 + ( r.x * cos(angle) - r.y * sin(angle) ),screenSize.y/2 + ( r.x * sin(angle) + r.y * cos(angle) ) };
		current = secr, starting = secr, previous = secr;
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
	void setPosition(const sf::Vector2f& n) {
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
	enum class LineModes {
		both=1,
		visual=2,
		collision=3
	};
	std::mutex queueMutex;
	std::vector<sf::VertexArray*> linesArray;
	std::vector<std::pair<float, float>> collisionPointMap;
	std::vector<std::pair<float, float>> collisionPointQueue;
	LineManager() {
		linesArray.reserve(100);
		collisionPointMap.reserve(10000);
		collisionPointQueue.reserve(100);
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
	virtual ~LineManager() {
		collisionPointQueue.clear();
		collisionPointMap.clear();
		linesArray.clear();
	}
	void setPathBoth() {
		
	}
};

enum class Inputs {
	Left = 1,
	Right = 2,
	Space = 3,
};

class Player : public LineManager, public PositionManager {
	bool placesPath = true;
	int size{};
	int id{};
	sf::Clock linerestart;
	LineModes lineMode = LineModes::both;
public:
	friend class MyRenderWindow;
	Player(sf::Vector2f p = { 400, 400 }, int s = 5) : PositionManager(p), LineManager() {
		size = s;
		placesPath = true;
		initiateLine();
	}
	Player(int radius = 300, int s = 5) : PositionManager(radius), LineManager() {
		size = s;
		placesPath = true;
		initiateLine();
	}
	void setLineMode(LineModes newMode) {
		lineMode = newMode;
	}
	int getId() {
		return id;
	}
	void moveTo(sf::Vector2f newp) {
		if (placesPath) {
			setPath();
			updateCollisionQueue();
		}
		setPosition(newp);
	}
	void moveBy(float distance) {
		if (placesPath) {
			setPath();
			updateCollisionQueue();
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
		for (std::pair<float, float> point : collisionPointMap) {
			if (distance(current, {point.first, point.second}) < size * 2) {
				return true;
			}
		}
		return false;
	}


	bool checkForCollision(Player& other) {
		// check for collision with screen border, then
		// check for collision with path
		if (current.x > screenSize.x ||
			current.x < 0 ||
			current.y > screenSize.y ||
			current.y < 0)
		{
			return true;
		}
		for (std::pair<float, float> point : other.collisionPointMap) {
			if (abs(point.first - getPosition().x) < size * 4) { // calculate only if near on x axis
				if (distance(current, {point.first, point.second}) < size * 2) {
					return true;
				}
			}
		}
		return false;
	}

	void updateCollisionQueue() {
		std::vector<std::pair<float, float>>::iterator queuedPoint = collisionPointQueue.begin();
		for (; queuedPoint != collisionPointQueue.end();) {
			if (distance(current, { queuedPoint->first, queuedPoint->second }) > size * 2) {
				collisionPointMap.push_back(*queuedPoint);
				//rescure p address;
				queuedPoint = collisionPointQueue.erase(queuedPoint);
			}
			else {
				//increment p address
				queuedPoint++;
			};
		}
	}
	int randRGBv0 = rand() % 255;
	int randRGBv1 = rand() % 255;
	int randRGBv2 = rand() % 255;

	void setVisualPath() {
		float angleFromPreviousPoint = getAngleFromPrevious();
		Vector c1(size, angleFromPreviousPoint + 1 * PI / 6);
		Vector c2(size, angleFromPreviousPoint - 1 * PI / 6);
		//didnt work I guess
		sf::Vector2f slope = { current.x / previous.x, current.y / previous.y };
		for (float i{}; i < 1; i++) {
			sf::Vertex v3 = sf::Vertex(current - (slope * i) + c1.getDisplacement(), sf::Color(randRGBv0, randRGBv1, randRGBv2, 255));
			sf::Vertex v4 = sf::Vertex(current - (slope * i) + c2.getDisplacement(), sf::Color(randRGBv0, randRGBv1, randRGBv2, 255));
			linesArray[getLineIndex()]->append(v3);
			linesArray[getLineIndex()]->append(v4);
		}

	}
	void setCollisionPath() {
		float angleFromPreviousPoint = getAngleFromPrevious();
		Vector c1(size, angleFromPreviousPoint + 1 * PI / 6);
		Vector c2(size, angleFromPreviousPoint - 1 * PI / 6);
		sf::Vertex v1 = sf::Vertex(previous + c1.getDisplacement(), sf::Color(255, 255, 0, 255));
		sf::Vertex v4 = sf::Vertex(current + c2.getDisplacement(), sf::Color(randRGBv0, randRGBv1, randRGBv2, 255));
		sf::Vector2f mid = midpoint(v1.position, v4.position);
		collisionPointQueue.push_back({ mid.x, mid.y });
	}
	void setPath() {
		if (lineMode == LineModes::both || lineMode == LineModes::visual) {
			setVisualPath();
		}
		if (lineMode == LineModes::both || lineMode == LineModes::collision) {
			setCollisionPath();
		}
	}
	~Player() {}
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
	sf::TcpSocket& socket;
	ControlSignals ctrl;
	sf::Clock clock;
	sf::Time timeSinceLastUpdate;
	bool movable{};
	NetworkPlayer(sf::TcpSocket& co) : Player(300), socket(co), ctrl() {
		clock.restart();
		movable = true;
	};

	void processMovement(sf::Packet& p) {
		timeSinceLastUpdate = clock.restart();
		bool left, right, space;
		p >> left >> right >> space;
		changeAngle(-1 * left * 0.003 * timeSinceLastUpdate.asMilliseconds());
		changeAngle(1 * right * 0.003 * timeSinceLastUpdate.asMilliseconds());
		moveBy(0.05 * timeSinceLastUpdate.asMilliseconds());
	}
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
	srand(time(NULL));

	// initiate game clock
	sf::Clock clock;
	float elapsed = 0;

	// initiate player and all that has to do with them
	// doing linesArray stuff directly within player object breaks everything ; . ;
	Player player(200, 2); //starting position, starting size
	Player player2(200, 2);

	//initiate keymap (used to negate keyboard input lag)
	std::map<sf::Keyboard::Key, bool> keymap;
	sf::VertexArray* arrow = new sf::VertexArray;

	//Make it all into a  game object
	auto restart = [&]() {
		player.restart();
		player2.restart();
	};

	std::vector<Player*> players;
	players.push_back(&player);
	players.push_back(&player2);

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
	player2.setSize(10);
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
					player2.restart();
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
		if (keymap[sf::Keyboard::J]) {
			player2.changeAngle(-0.00001 * elapsed);
		}
		if (keymap[sf::Keyboard::L]) {
			player2.changeAngle(0.00001 * elapsed);
		}
		doDebug = keymap[sf::Keyboard::B];
		player.setPlacesPath(!keymap[sf::Keyboard::Space]); //tutaj
		player.moveBy(0.0002 * elapsed);
		player2.moveBy(0.0002 * elapsed);
		bool ifFound = false;
		for (Player* p : players) {
			for (Player* q : players) {
				if (p->checkForCollision(*q)) {
					restart();
					ifFound = true;
					continue;
				}
			}
			if (ifFound == true) { continue; }
		}
		if (ifFound == true) { continue; }
		window.clear();

		window.draw(player);
		window.draw(player2);
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

void menu(MyRenderWindow& window, std::atomic<State>& s) {
	ImGui::Begin("D");

	// background tries
	/*sf::RectangleShape background;
	background.setSize(sf::Vector2f(400, 400));
	sf::Texture maintexture;
	maintexture.loadFromFile("CurveFever/Texture/gg.jpeg");
	background.setTexture(&maintexture);*/

	if (ImGui::Button("start")) {
		s = State::singleplayer;
	}
	if (ImGui::Button("multiplayer")) {
		s = State::multiplayerMenu;
	}
	ImGui::Text("testest");
	ImGui::End();

	//window.draw(background);

	window.clear();
	ImGui::SFML::Render(window);
	window.display();
}

enum class networkFlags {
	JOIN = 1,
	DISCONNECT = 2,
	TEST = 3,
};

void emptyFn() {};

class networkClient {
	sf::IpAddress ip;
	std::size_t recsize{};
	bool isConnected = false;
	bool isConnecting = false;
	sf::TcpSocket socket;
	std::string w = "welcome";
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
		sf::Packet incomingMessage;
		socket.receive(incomingMessage);
		std::string data;
		incomingMessage >> data;
		if (data == w) {
			std::cout << "CONNECTED!" << endl;
			isConnected = true;
		}
		isConnecting = false;
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
	std::unique_ptr<std::thread> waiterThread;
	void awaitStart(std::atomic<State>& s) {
		sf::Packet p;
		std::string procedure;
		while (true) {
			socket.receive(p);
			p >> procedure;
			cout << "proc" << procedure << endl;
			if (procedure == "START") {
				s = State::multiplayer;
				break;
			}
		}
	};
	void join(std::atomic<State>& s) {
		sf::Packet p;
		std::string t = "JOIN";
		p << t;
		socket.send(p);
		waiterThread.reset(new std::thread([&]() { awaitStart(s); }));
	}
	void start() {
		sf::Packet p;
		std::string t = "START";
		p << t;
		socket.send(p);
	}
	sf::TcpSocket& getSocket() {
		return socket;
	}
};


// length of the string lol
int len(std::string str)
{
	int length = 0;
	for (int i = 0; str[i] != '\0'; i++)
	{
		length++;
	}
	return length;
}
/*
	<b>Clears</b> the vector and fills it with sliced string
*/
void splitTo(std::string str, const char seperator, std::vector<std::string>& cont)
{
	cont.clear();
	int currIndex = 0, i = 0;
	int startIndex = 0, endIndex = 0;
	while (i <= len(str))
	{
		if (str[i] == seperator || i == len(str))
		{
			endIndex = i;
			std::string subStr = "";
			subStr.append(str, startIndex, endIndex - startIndex);
			cont.push_back(subStr);
			currIndex += 1;
			startIndex = endIndex + 1;
		}
		i++;
	}
}


std::pair<std::string, std::string> splitOnceBy(std::string str, const char seperator)
{
	int currIndex = 0, i = 0;
	int startIndex = 0, endIndex = 0;
	std::pair<std::string, std::string> result;
	while (i <= len(str))
	{
		if (str[i] == seperator || i == len(str))
		{
			endIndex = i;
			std::string subStr = "";
			subStr.append(str, startIndex, endIndex - startIndex);
			result.first = subStr;
			subStr = "";
			subStr.append(str, endIndex - startIndex);
			result.second = subStr;
			return result;
		}
		i++;
	}
	result.first = str;
	return result;
}


void multiplayer(MyRenderWindow& window, std::atomic<State>& s, networkClient& net) {
	srand(time(NULL));
	bool isRunning = true;
	bool isStarted = false;
	std::vector<std::shared_ptr<Player>> players;
	auto processMovementFromString = [&](std::string serializedData) {
		std::vector<std::string> playerData;
		splitTo(serializedData, '|', playerData);
		for (int i = 0; i < playerData.size(); i++) {
			std::vector<std::string> playerMovement;
			splitTo(playerData[i], ' ', playerMovement);
			if (playerMovement.size() >= 3) {
				players[i]->setPlacesPath(playerMovement[0] == "1");
				sf::Vector2f newPosition = {
						(float)std::atof(playerMovement[1].c_str()),
						(float)std::atof(playerMovement[2].c_str())
				};
				players[i]->moveTo(newPosition);
			}
		}
	};
	auto createPlayers = [&](std::string serializedData) {
		std::vector<std::string> playerData;
		std::vector<std::string> playerMovement;
		splitTo(serializedData, '|', playerData);
		for (int i = 0; i < playerData.size(); i++) {
			splitTo(playerData[i], ' ', playerMovement);
			//There is a white space at the end of every serialized string...
			if (playerMovement.size() >= 3) {
				bool pathState = playerMovement[0] == "1";
				sf::Vector2f newPosition = {
						(float)std::atof(playerMovement[1].c_str()),
						(float)std::atof(playerMovement[2].c_str())
				};
				std::shared_ptr<Player> p = std::make_shared<Player>(newPosition, 8);
				p->setPlacesPath(pathState);
				p->setLineMode(LineManager::LineModes::visual);
				players.push_back(p);
			}
		}
	};

	std::thread recvLoopThread([&]() {
		while (isRunning) {
			sf::Packet pac1;
			net.getSocket().receive(pac1);
			std::string procedure;
			std::string stringifiedData;
			pac1 >> procedure >> stringifiedData;
			if (procedure == "UPDATE") {
				if (!isStarted) {
					createPlayers(stringifiedData);
					isStarted = true;
				}
				else {
					processMovementFromString(stringifiedData);
				}
			}
		}
		});
	std::thread sendLoopThread([&]() {
		while (isRunning) {
			sf::Packet pac2;
			bool right = net.keymap[sf::Keyboard::A], left = net.keymap[sf::Keyboard::D], space = net.keymap[sf::Keyboard::Space];
			std::string procedure = "UPDATE";
			pac2 << procedure << left << right << space;
			std::cout << procedure << left << right << space << std::endl;
			net.getSocket().send(pac2);
			std::this_thread::sleep_for(std::chrono::milliseconds(1000)/100);
		}
		});
	while (window.isOpen()) {
		sf::Event event;
		while (window.pollEvent(event)) {
			switch (event.type) {
			case(sf::Event::Closed): {
				isRunning = false;
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
		for (auto& x : players) {
			window.draw(*x);
		}
		window.display();
	}
	recvLoopThread.join();
	sendLoopThread.join();
}
void multiplayerMenu(MyRenderWindow& window, std::atomic<State>& s, networkClient& net) {
	if (ImGui::Button("back")) {
		s = State::menu;
		net.disconnect();
	}
	if (ImGui::Button("start")) {
		net.start();
	}

	if (ImGui::Button("join")) {
		net.join(s);
	}

	if (ImGui::Button("test")) {
		net.test();
	}

	window.clear();
	ImGui::SFML::Render(window);
	window.display();
}

void multiplayerConnecting(MyRenderWindow& window, std::atomic<State>& s, networkClient& net) {
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

bool compareHosts(sf::TcpSocket& c, sf::TcpSocket& s) {
	return (
		c.getLocalPort() == c.getLocalPort() && c.getRemoteAddress() == s.getRemoteAddress() && c.getRemotePort() == s.getRemotePort());
}

class Server2ndTry {
	sf::TcpListener listener;
	sf::SocketSelector selector;
	std::mutex socketMutex;
	std::vector<std::shared_ptr<sf::TcpSocket>> sockets;
	std::vector<std::shared_ptr<NetworkPlayer>> players;
	sf::Clock clock;
	bool started = false;
	bool pathsStarted = false;
	
public:
	void start() {
		clock.restart();
		std::thread t([&]() {acceptLoop();});
		std::thread y([&]() {recvLoop();});
		std::thread z([&]() {updateLoop();});
		while (true) {}
	}
	std::string serializePlayerData() {
		std::stringstream stream;
		for (auto& player : players) {
			stream << player->getPlacesPath();
			stream << " ";
			stream << std::fixed << player->getPosition().x;
			stream << " ";
			stream << std::fixed << player->getPosition().y;
			stream << "|";
		}
		return stream.str();
	}
	void handleStart(sf::Packet& p, sf::TcpSocket& s) {
		sf::Packet ps;
		if (started) {
			return;
		}
		ps << "START";
		started = true;
		for (auto& player : players) {
			cout << "Sent start" << endl;
			player->clock.restart();
			player->socket.send(ps);
		}
		clock.restart();

	}
	void startPathsIf(bool conditions) {
		if (!pathsStarted && conditions) {
			cout << "PATH" << endl;
			for (auto& player : players) {
				player->setPlacesPath(true);
			}
			pathsStarted = true;
		}
	}
	
	void handleUpdate(sf::Packet& incomingMessage, sf::TcpSocket& socket) {
		startPathsIf(clock.getElapsedTime().asSeconds() > 2);
		for (int i{}; i < players.size(); i++) {
			if (compareHosts(socket, players[i]->socket)) {
				for (int j = i; j < players.size(); j++) {
					if (players[i]->movable && players[i]->checkForCollision(*players[j])) {
						players[i]->movable = false;
					}
				}
				if(players[i]->movable) 
					players[i]->processMovement(incomingMessage);
			}
		}
	}

	void updateLoop() {
		while (true) {
			if (started) {
				sf::Packet ps;
				ps << "UPDATE";
				ps << serializePlayerData();
				for (auto& player : players) {
					player->socket.send(ps);
				}
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(1000)/100);
		}
	}
	void handleJoin(sf::Packet& IncomingMessage, sf::TcpSocket& socket) {
		sf::Packet outgoingMessage;
		std::cout << "JOIN" << std::endl;
		bool found = false;
		for (auto& player : players) {
			if (compareHosts(player->socket, socket)) {
				found = true;
				break;
			}
		}
		if (!found) {
			std::shared_ptr<NetworkPlayer> p = std::make_shared<NetworkPlayer>(socket);
			p->setPlacesPath(false);
			p->setLineMode(LineManager::LineModes::collision);
			players.push_back(p);
			outgoingMessage << "JOIN";
		}
		else {
			outgoingMessage << "JOIN FAILED";
		
		}
		socket.send(outgoingMessage);
	}

	void parseRecieved(sf::Packet& incomingMessage, sf::TcpSocket& socket) {
		std::string procedure;
		incomingMessage >> procedure;
		if (procedure == "START")
			handleStart(incomingMessage, socket);
		if (procedure == "UPDATE")
			handleUpdate(incomingMessage, socket);
		if (procedure == "JOIN")
			handleJoin(incomingMessage, socket);
	}

	void acceptLoop() {
		while (true) {
			std::shared_ptr<sf::TcpSocket> client = std::make_shared<sf::TcpSocket>();
			sf::Packet outgoingMessage;
			outgoingMessage << "welcome";
			if (listener.listen(53000) != sf::Socket::Done) {
				std::cout << "Failed to listen on port" << std::endl;
			}
			if (listener.accept(*client) != sf::Socket::Done) {
				std::cout << "Failed to accept the client" << std::endl;
			}
			socketMutex.lock();
			sockets.push_back(client);
			client->send(outgoingMessage);
			selector.add(*client);
			std::cout << "Client accepted" << endl;
			socketMutex.unlock();
		}
	}
	void recvLoop() {
		while (true) {
			if (selector.wait(sf::seconds(10.f))) {
				for (auto iter = sockets.begin(); iter != sockets.end();) {
					socketMutex.lock();
					sf::TcpSocket& soc = **iter;
					if (selector.isReady(soc)) {
						sf::Packet p;
						sf::Socket::Status s = soc.receive(p);
						if (s == sf::Socket::Status::Disconnected)
						{
							selector.remove(soc);
							iter = sockets.erase(iter);
						}	
						else {
							parseRecieved(p, soc);
							iter++;
						};
					}
					else
						iter++;
					socketMutex.unlock();
					}
				}
			 else
				std::cout << "timeout" << std::endl;
		}
	};
};

void server2ndTry() {
	Server2ndTry s;
	s.start();
}

void client() {
	sf::ContextSettings settings;
	settings.antialiasingLevel = 20.0;
	MyRenderWindow window(sf::VideoMode(screenSize.x, screenSize.y), "SFML", settings);
	window.setFramerateLimit(60);
	ImGui::SFML::Init(window);
	networkClient net;
	/*
		choice of game mode - single player, multi player
	*/
	std::atomic<State> currentState = State::menu;
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

int main() {
	srand(time(NULL));
	char varstart;
	cout << "Enter 'y': " << endl;
	std::cin >> varstart;

	if (varstart == 'y') {
		client();
	}
	else {
		server2ndTry();
	}
}
