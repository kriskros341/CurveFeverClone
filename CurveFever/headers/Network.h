#pragma once
#include "Player.h"
#include <thread>
#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <mutex>
#include <sstream>
const std::string defaultPort = "5030";

enum class State {
	singleplayer = 1,
	multiplayerMenu,
	menu,
	multiplayer,
	serverHost
};

bool compareHosts(sf::TcpSocket& c, sf::TcpSocket& s);

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
	void restart() {
		movable = true;
		clock.restart();
		Player::restart();
	}
	void processMovement(sf::Packet& p);
};


class networkClient {
	std::size_t recsize{};
	bool isConnected = false;
	bool isConnecting = false;
	sf::TcpSocket socket;
	std::string w = "welcome";
	std::unique_ptr<std::thread> waiterThread;
public:
	sf::IpAddress ip;
	std::string port = "5030";
	std::map<sf::Keyboard::Key, bool> keymap;
	networkClient() {
		//ip = sf::IpAddress::getLocalAddress();
		port = defaultPort;
		ip = sf::IpAddress::getPublicAddress(sf::seconds(5.0f));
	}
	bool getConnected();	
	void setIpAddress(std::string s) {
		ip = sf::IpAddress(s);
	}
	void setPort(std::string p) {
		port = p;
	}
	bool getConnecting();	
	bool isWorking();	
	void connect();
	void cancelConnect();	
	void disconnect();	
	void test();	
	void awaitStart(std::atomic<State>& s);	
	void join(std::atomic<State>& s);
	void start();
	sf::TcpSocket& getSocket();
};


class Server2ndTry {
	sf::TcpListener listener;
	sf::SocketSelector selector;
	std::mutex socketMutex;
	std::vector<std::shared_ptr<sf::TcpSocket>> sockets;
	std::vector<std::shared_ptr<NetworkPlayer>> players;
	sf::Clock clock;
	bool started = false;
	bool pathsStarted = false;
	std::atomic<bool> isRunning;
	std::thread loops;
	
	int currentRound = 0, roundLimit = 4;
	void startNewRound() {
		pathsStarted = false;
		clock.restart();
		sf::Packet message;
		std::stringstream stream;
		message << "RESTART";
		for (auto& p : players) {
			p->score.nextRound();
			p->restart();
			p->movable = true;
			p->setPlacesPath(false);
		}
		for (auto& p : players) {
			stream << p->getPosition().x << " " << p->getPosition().y << " " << p->movable << "|";
		}
		std::string temp = stream.str();
		temp.pop_back(); // removing the last |
		message << temp;
		for (auto& p : players) {
			p->socket.send(message);
		}
	}
	void endGame() {
		sf::Packet message;
		std::stringstream stream;
		message << "SCORE";
		for (auto& p : players) {
			stream << p->score.getScore() << " ";
		}

		std::string temp = stream.str();
		temp.pop_back();
		message << temp;
		for (auto& p : players) {
			p->socket.send(message);
		}

	}
public:
	void start();
	bool isVictoryConditionMet();
	std::string serializePlayerData();
	void handleStart(sf::Packet& p, sf::TcpSocket& s);
	void startPathsIf(bool conditions);
	void handleUpdate(sf::Packet& incomingMessage, sf::TcpSocket& socket);
	void updateLoop();
	void handleJoin(sf::Packet& IncomingMessage, sf::TcpSocket& socket);
	void parseRecieved(sf::Packet& incomingMessage, sf::TcpSocket& socket);
	void acceptLoop();
	void restart();
	void restartRound();
	void recvLoop();
	void stopServer();
	void setRunning(bool newState) {
		return isRunning.store(newState);
	}
	bool getRunning() {
		return isRunning.load();
	}
};
