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
#include <Player.h>
#include <Window.h>
#include <Network.h>
#define PI std::acos(0) * 2

extern enum class State;

extern const sf::Vector2u screenSize;
using std::cout; using std::endl;


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



void menu(MyRenderWindow& window, std::atomic<State>& s, BackgroundImage& bcgg) {
	ImGui::Begin("D");

	if (ImGui::Button("start")) {
		s = State::singleplayer;
	}
	if (ImGui::Button("multiplayer")) {
		s = State::multiplayerMenu;
	}
	ImGui::Text("testest");
	ImGui::End();

	window.clear();
	window.draw(bcgg);
	ImGui::SFML::Render(window);
	window.display();
}

enum class networkFlags {
	JOIN = 1,
	DISCONNECT = 2,
	TEST = 3,
};

void emptyFn() {};



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
	BackgroundImage bcgg;

	//choice of game mode - single player, multi player
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
			menu(window, currentState, bcgg);
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
