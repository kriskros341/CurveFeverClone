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

sf::Font font;
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

	//initiate game clock
	sf::Clock clock;
	float elapsed = 0;

	// initiate player and all that has to do with them
	// doing linesArray stuff directly within player object breaks everything ; . ;
	Player player(200, 2); //starting position, starting size
	Player player2(200, 2);
	int score1{}, score2{};
	int roundLimit = 5;
	int currentRound = 0;

	//initiate keymap (used to negate keyboard input lag)
	std::map<sf::Keyboard::Key, bool> keymap;
	sf::VertexArray* arrow = new sf::VertexArray;

	//Make it all into a  game object
	auto restart = [&]() {
		currentRound += 1;
		score1 += player.score.getScore();
		player.restart();
		score2 += player2.score.getScore();
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
			player.changeAngle(-0.000005 * elapsed);
		}
		if (keymap[sf::Keyboard::D]) {
			player.changeAngle(0.000005 * elapsed);
		}
		if (keymap[sf::Keyboard::J]) {
			player2.changeAngle(-0.000005 * elapsed);
		}
		if (keymap[sf::Keyboard::L]) {
			player2.changeAngle(0.000005 * elapsed);
		}
		doDebug = keymap[sf::Keyboard::B];
		//player.setPlacesPath(!keymap[sf::Keyboard::Space]); //tutaj
		if (currentRound < roundLimit) {
			player.moveBy(0.00015 * elapsed);
			player2.moveBy(0.00015 * elapsed);
		}
		bool ifFound = false;
		for (Player* p : players) {
			for (Player* q : players) {
				if (p->checkForCollision(*q)) {
					q->score.addScore(20);
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

		sf::Text t1;
		t1.setString(std::to_string(score1 + player.score.getScore()));
		t1.setCharacterSize(40);
		t1.setFont(font);
		t1.setPosition({ 50, 50 });
		t1.setFillColor({ (sf::Uint8)player.randRGBv0, (sf::Uint8)player.randRGBv1, (sf::Uint8)player.randRGBv2 });
		window.draw(t1);
		sf::Text t2;
		t2.setString(std::to_string(score2 + player.score.getScore()));
		t2.setCharacterSize(40);
		t2.setFont(font);
		t2.setPosition({ 50, 100 });
		t2.setFillColor({ (sf::Uint8)player2.randRGBv0, (sf::Uint8)player2.randRGBv1, (sf::Uint8)player2.randRGBv2 });
		window.draw(t2);
		if(doDebug) debug();
		window.display();
		// increment game tick by a value modified by time between frames
		// this is supposed to make gameplay independent of frames per second and ping in the fututre

		currentTick = currentTick + 5 * elapsed;
	}
	delete arrow;
}



void menu(MyRenderWindow& window, std::atomic<State>& s, BackgroundImage& bcgg) {
	ImGui::Begin("Menu");
	ImGuiStyle* style = &ImGui::GetStyle();

	style->WindowBorderSize = 0;
	style->WindowTitleAlign = ImVec2(0.5, 0.5);
	style->WindowMinSize = ImVec2(300, 300);
	style->Colors[ImGuiCol_TitleBg] = ImColor(255, 101, 53, 255);
	style->Colors[ImGuiCol_TitleBgActive] = ImColor(255, 101, 53, 255);
	style->Colors[ImGuiCol_TitleBgCollapsed] = ImColor(0, 0, 0, 200);

	style->Colors[ImGuiCol_Button] = ImColor(31, 30, 31, 255);
	style->Colors[ImGuiCol_ButtonActive] = ImColor(31, 30, 31, 255);
	style->Colors[ImGuiCol_ButtonHovered] = ImColor(41, 40, 41, 255);

	ImGui::Spacing();
	if (ImGui::Button("Local multiplayer", ImVec2(285, 50))) {
		s = State::singleplayer;
	}
	ImGui::Spacing();
	if (ImGui::Button("Server multiplayer", ImVec2(285, 50))) {
		s = State::multiplayerMenu;
	}
	ImGui::Spacing();
	if (ImGui::Button("Host", ImVec2(285, 50))) {
		s = State::serverHost;
	}
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


std::mutex tempMut;

void multiplayer(MyRenderWindow& window, std::atomic<State>& s, networkClient& net) {
	srand(time(NULL));
	bool isRunning = true;
	bool isStarted = false;
	bool showScore = false;
	
	std::vector<std::string> scores;
	std::vector<std::shared_ptr<Player>> players;
	auto processMovementFromString = [&](std::string serializedData) {
		std::vector<std::string> playerData;
		splitTo(serializedData, '|', playerData);
		tempMut.lock();
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
		tempMut.unlock();
	};
	auto createPlayers = [&](std::string serializedData) {
		std::vector<std::string> playerData;
		std::vector<std::string> playerMovement;
		splitTo(serializedData, '|', playerData);
		tempMut.lock();
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
		tempMut.unlock();
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
			if (procedure == "SCORE") {
				if (!showScore) {
					splitTo(stringifiedData, ' ', scores);
					showScore = true;
				}
			}
			if (procedure == "RESTART") {
				std::vector<std::string> playerData;
				splitTo(stringifiedData, '|', playerData);
				//std::cout << stringifiedData << " " << playerData.size() << std::endl;;
				tempMut.lock();
				for (int i = 0; i < playerData.size(); i++) {
					players[i]->restart();
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
				tempMut.unlock();
			}
		}
		});
	std::thread sendLoopThread([&]() {
		while (isRunning) {
			sf::Packet pac2;
			bool right = net.keymap[sf::Keyboard::A], left = net.keymap[sf::Keyboard::D], space = net.keymap[sf::Keyboard::Space];
			std::string procedure = "UPDATE";
			pac2 << procedure << left << right << space;
			//std::cout << procedure << left << right << space << std::endl;
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
		tempMut.lock();
		for (auto& x : players) {
			window.draw(*x);
		}
		for (int i{}; i < scores.size(); i++) {
			sf::Text t;
			t.setString(scores[i]);
			t.setCharacterSize(40);
			t.setFont(font);
			t.setPosition({ 50, (float)50*i });
			t.setFillColor({ (sf::Uint8)players[i]->randRGBv0, (sf::Uint8)players[i]->randRGBv1, (sf::Uint8)players[i]->randRGBv2 });
			window.draw(t);
		}
		tempMut.unlock();
		window.display();
	}
	recvLoopThread.join();
	sendLoopThread.join();
}
void multiplayerMenu(MyRenderWindow& window, std::atomic<State>& s, networkClient& net) {
	static char address[255]{};
	static char nickname[255]{};
	ImGui::InputText("Ip Address", address, sizeof(address));
	ImGui::InputText("Nickname", nickname, sizeof(nickname));
	net.ip = sf::IpAddress(address);
	static int connectionState = 0;
	static std::vector<std::string> leaderboardData;
	if (ImGui::Button("back")) {
		s = State::menu;
		net.disconnect();
	}
	switch (connectionState) {
	case 0: {
		if (ImGui::Button("Connect")) {
			std::cout << "attempting to connect to " << net.ip << " at " << net.port << std::endl;
			net.connect();
		}
		if (net.isWorking()) {
			connectionState = 1;
		}
		break;
	}
	case 1: {
		if (ImGui::Button("join")) {
			net.join(s, nickname);
			std::string sc = net.getLeaderboardData(); 
			splitTo(sc, ' ', leaderboardData);
			std::cout << sc;
			connectionState = 2;
		}
		if (ImGui::Button("test")) {
		}
		break;
	}
	case 2:
		if (ImGui::Button("start")) {
			net.start();
		}
		if (ImGui::Button("Leave")) {
			net.disconnect();
			connectionState = 0;
		}
	}
	window.clear();

	for (int i{}; i < leaderboardData.size(); i++) {
		std::cout << leaderboardData[i] << std::endl;
		sf::Text t;
		t.setString(leaderboardData[i]);
		t.setCharacterSize(40);
		t.setFont(font);
		t.setFillColor(sf::Color::White);
		t.setPosition({ 200, (float)50*i+50 });
		window.draw(t);
	}
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


void server2ndTry(MyRenderWindow& window, std::atomic<State>& gameState) {
	static std::unique_ptr<Server2ndTry> s = std::make_unique<Server2ndTry>();
	static std::string publicIpAddress = sf::IpAddress::getPublicAddress(sf::seconds(5.0f)).toString();
	static std::thread serverThread;
	std::string localIpAddress = sf::IpAddress::getLocalAddress().toString();
	int port = std::atoi(defaultPort.c_str());
	std::stringstream text;
	text << "awaiting messages at " << publicIpAddress << " publicly and " << localIpAddress << " Locally at " << port;
	std::string debugMessage = text.str();
	
	ImGui::Begin("tototototottototojesttest");
	bool isRunning = s->getRunning();
		if (isRunning) {
			ImGui::Text(debugMessage.c_str());
			if (ImGui::Button("stop")) {
				s->stopServer();
			}
			ImGui::Text("Make sure your firewall doesn't block the communication and that port is open");
		}
		else {
			if (ImGui::Button("back")) {
				gameState = State::menu;
			}
			if (ImGui::Button("start")) {
				s->getLeaderboardData();
				s->start();
			}
		}
	ImGui::End();
	window.clear();
	ImGui::SFML::Render(window);
	window.display();
}


void client() {
	sf::ContextSettings settings;
	settings.antialiasingLevel = 20.0;
	MyRenderWindow window(sf::VideoMode(screenSize.x, screenSize.y), "SFML", settings);
	window.setFramerateLimit(60);
	ImGui::SFML::Init(window);
	font.loadFromFile("Arial.ttf");
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
			multiplayerMenu(window, currentState, net);
			break;
		}
		case State::serverHost: {
			server2ndTry(window, currentState);
			break;
		}
		}
	}
	ImGui::SFML::Shutdown();
}

int main() {
	srand(time(NULL));
	client();
}
