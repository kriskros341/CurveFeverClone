#pragma once
#include "Player.h"
#include <thread>
#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <mutex>
#include <sstream>

enum class State {
	singleplayer = 1,
	multiplayerMenu,
	menu,
	multiplayer
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
	void processMovement(sf::Packet& p);
};


class networkClient {
	sf::IpAddress ip;
	std::size_t recsize{};
	bool isConnected = false;
	bool isConnecting = false;
	sf::TcpSocket socket;
	std::string w = "welcome";
	std::unique_ptr<std::thread> waiterThread;
public:
	std::map<sf::Keyboard::Key, bool> keymap;
	networkClient() {
		//ip = sf::IpAddress::getLocalAddress();
		ip = sf::IpAddress("83.26.49.174");
	}
	bool getConnected();	
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
public:
	void start();
	std::string serializePlayerData();
	void handleStart(sf::Packet& p, sf::TcpSocket& s);
	void startPathsIf(bool conditions);
	void handleUpdate(sf::Packet& incomingMessage, sf::TcpSocket& socket);
	void updateLoop();
	void handleJoin(sf::Packet& IncomingMessage, sf::TcpSocket& socket);
	void parseRecieved(sf::Packet& incomingMessage, sf::TcpSocket& socket);
	void acceptLoop();
	void recvLoop();
};
