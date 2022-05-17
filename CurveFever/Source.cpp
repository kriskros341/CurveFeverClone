#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <iostream>
#include <cmath>
#include <csignal>
#include <map>
#include "imgui.h"
#include "imgui-SFML.h"

#define PI std::acos(0) * 2
sf::Vector2u screenSize(800, 800);
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

class ControlManager {
	/*
		How each player accesses data about their movement.

		Network would create a tcp listener that updates everything based on packets it recieves
		Keyboard would recieve keyboard updates from a seperate thread

	*/
};

class NetworkControl : public ControlManager {

};

//OPTIONAL
class AIControl: public ControlManager {

};

class KeyboardControl: public ControlManager {

};

class Player : public LineManager, public PositionManager {
	bool placesPath = true;
	int size{};
public:
	friend class MyRenderWindow;
	Player(sf::Vector2f p, int s) : PositionManager(p), LineManager() {
		size = s;
		placesPath = true;
		initiateLine();
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

	bool checkForCollision(Player* other) {
		// check for collision with screen border, then
		// check for collision with path
		if (current.x > screenSize.x ||
			current.x < 0 ||
			current.y > screenSize.y ||
			current.y < 0)
		{
			return true;
		}
		for (auto p = other->collisionPointMap.begin(); p < other->collisionPointMap.end(); ++p) {
			if (distance(current, { p->first, p->second }) < size * 2) {
				//collision occured
				return true;
			}
		}
		return false;
	}

	int randRGBv0 = rand() % 255;
	int randRGBv1 = rand() % 255;
	int randRGBv2 = rand() % 255;

	void setPath() {

		// I think it can be done better
		float angleFromPreviousPoint = getAngleFromPrevious();
		Vector c1(size, angleFromPreviousPoint + 1 * PI / 6);
		Vector c2(size, angleFromPreviousPoint - 1 * PI / 6);
		
		// for loop for (heh) generating random color from 0 to 255
		// dunno how to assign number which will be constant through one whole gameplay

		/*for (int i{}; i < 6; i++)
		{
			srand(time(NULL));
			randRGBv[i] = rand() % 255;
			cout << randRGBv[i];
		}*/

		// create a rectangle
		sf::Vertex v1 = sf::Vertex(previous + c1.getDisplacement(), sf::Color(255, 255, 0, 255));
		sf::Vertex v2 = sf::Vertex(previous + c2.getDisplacement(), sf::Color(255, 255, 0, 255));
		sf::Vertex v3 = sf::Vertex(current + c1.getDisplacement(), sf::Color(randRGBv0, randRGBv1, randRGBv2, 255));
		sf::Vertex v4 = sf::Vertex(current + c2.getDisplacement(), sf::Color(randRGBv0, randRGBv1, randRGBv2, 255));
		
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

class MyRenderWindow : public sf::RenderWindow {
public:
	MyRenderWindow(sf::VideoMode v, std::string title, sf::ContextSettings c) : sf::RenderWindow(v, title, sf::Style::Close, c) {}
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

int server() {
	sf::TcpListener listener;
	if (listener.listen(53000) != sf::Socket::Done) {
		std::cout << "JD" << std::endl;
	}
	sf::TcpSocket client;
	if (listener.accept(client) != sf::Socket::Done) {
		std::cout << "CD" << std::endl;
	}

	return 0;
}


int client() {
	std::cout << "g" << std::endl;
	sf::TcpSocket socket;
	if (socket.connect("127.0.0.1", 53000) != sf::Socket::Done)
	{
		std::cout << "stuff" << std::endl;
	}
	std::cout << "jg" << std::endl;
	return 0;
}

void singleplayer(MyRenderWindow& window) {
	float currentTick{};
	float posX{}, posY{};
	
	// initiate game clock
	sf::Clock clock;
	float elapsed = 0;

	// random generated starting position
	srand(time(NULL));
	posX = rand() % 400 + 40;
	posY = rand() % 400 + 40;
	cout << posX << ", " << posY << endl;

	// initiate player and all that has to do with them
	// doing linesArray stuff directly within player object breaks everything ; . ;
	Player player({posX, posY}, 2); //starting position, starting size
	Player player2({ posX + 50, posY + 50 }, 2);

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
				// color changing prototype
				if (sf::Keyboard::isKeyPressed(sf::Keyboard::C)) {
					cout << "C" << endl;
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
		player.setPlacesPath(!keymap[sf::Keyboard::Space]);
		// Move player
		// Do not touch without a commit. Memory safe, but very fragile;
		player.moveBy(0.0002 * elapsed);
		player2.moveBy(0.0002 * elapsed);
		/*
		for gracz in gracze:
			for graczz in gracze:
				if gracz.checkcoll(graczz):
					restart()
		*/
		bool ifFound = false;
		for (Player* p : players) {
			for (Player* q : players) {
				if (p->checkForCollision(q)) {
					restart();
					ifFound = true;
					continue;
				}
			}
			if (ifFound == true) {continue;}
		}
		/*if (player.checkForCollision()) {
			restart();
			continue;
		}
		if (player2.checkForCollision()) {
			restart();
			continue;
		}*/
		//draw stuff
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
};

void menu(MyRenderWindow& window, State& s) {
	/*sf::RectangleShape button1({400, 200});
	sf::RectangleShape button2({400, 200});
	button2.setFillColor(sf::Color::Red);
	button2.setPosition(0, 200);*/
	ImGui::SFML::Init(window);
	sf::Clock deltaClock;
	bool ifSingle = false;

	while (window.isOpen()) {
		sf::Event event;
		while (window.pollEvent(event)) {
			sf::Vector2i mousePosition = sf::Mouse::getPosition(window);

			ImGui::SFML::ProcessEvent(event);

			//switch (event.type) {
			//case(sf::Event::Closed): {
			//	window.close();
			//}
			//case(sf::Event::MouseButtonPressed): {
			//	// pause on button press
			//	while (sf::Mouse::isButtonPressed(sf::Mouse::Right)) {}
			//	break;
			//}
			//case(sf::Event::MouseButtonReleased): {
			//	if (button1.getGlobalBounds().contains({ (float)mousePosition.x, (float)mousePosition.y })) {
			//		s = State::singleplayer;
			//		std::cout << "G" << std::endl;
			//	}
			//	if (button2.getGlobalBounds().contains({ (float)mousePosition.x, (float)mousePosition.y })) {
			//		s = State::multiplayerMenu;
			//	}
			//	break;
			//}
			//}

			//erased it for now but left for insight
		}
		if (s != State::menu)
			break;
		//draw stuff

		ImGui::SFML::Update(window, deltaClock.restart());
		ImGui::Begin("CurveFever", 0, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings);
		ImGui::Text("Hraj");
		ImGui::Checkbox("Singleplayer", &ifSingle);
		ImGui::End();
		// ImGui provisoric menu thing

		window.clear();
		if (ifSingle) {
			s = State::singleplayer;
		}
		/*window.draw(button1);
		window.draw(button2);*/

		ImGui::SFML::Render(window);

		window.display();
		// increment game tick by a value modified by time between frames
		// this is supposed to make gameplay independent of frames per second and ping in the fututre
	}

	ImGui::SFML::Shutdown();
}

void multiplayerMenu(MyRenderWindow& window, State& s) {
	sf::RectangleShape button1({400, 200});
	sf::RectangleShape button2({400, 200});
	button2.setFillColor(sf::Color::Red);
	button2.setPosition(0, 200);

	auto startServer = [&]() {

	};
	auto joinServer = [&]() {

	};
	while (window.isOpen()) {
		sf::Event event;
		while (window.pollEvent(event)) {
			sf::Vector2i mousePosition = sf::Mouse::getPosition(window);
			switch (event.type) {
			case(sf::Event::Closed): {
				window.close();
			}
			case(sf::Event::MouseButtonPressed): {
				// pause on button press
				while (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {}
				break;
			}
			case(sf::Event::MouseButtonReleased): {
				if (button1.getGlobalBounds().contains({ (float)mousePosition.x, (float)mousePosition.y })) {
					startServer();
				}
				if (button2.getGlobalBounds().contains({ (float)mousePosition.x, (float)mousePosition.y })) {
					joinServer();
				}
				break;
			}
			}
		}
		if (s != State::multiplayerMenu)
			break;
		//draw stuff
		window.clear();
		window.draw(button1);
		window.draw(button2);
		window.display();
		// increment game tick by a value modified by time between frames
		// this is supposed to make gameplay independent of frames per second and ping in the fututre
	}
	
}




int main() {
	// initiate window and globally used values
	srand(time(NULL));
	sf::ContextSettings settings;
	settings.antialiasingLevel = 20.0;
	MyRenderWindow window(sf::VideoMode(screenSize.x, screenSize.y), "SFML", settings);
	window.setFramerateLimit(60);
	
	/*
		choice of game mode - single player, multi player
	*/

	State currentState = State::menu;
	while (window.isOpen()) {
		switch (currentState) {
		case State::menu: {
			menu(window, currentState);
			break;
		};
		case State::singleplayer: {
			singleplayer(window);
			break;
		}
		case State::multiplayerMenu: {
			multiplayerMenu(window, currentState);
			break;
		}
		}
	}
	window.close();
		
}
