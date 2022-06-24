#pragma once
#include "Player.h"
#include <fstream>
#include <thread>
#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <mutex>
#include <algorithm>
#include <sstream>
const std::string defaultPort = "5030";
using playerScore = std::pair<std::string, int>;

enum class State {
	singleplayer = 1,
	multiplayerMenu,
	menu,
	multiplayer,
	serverHost
};
std::pair<std::string, std::string> splitOnceBy(std::string str, const char seperator);
void splitTo(std::string str, const char seperator, std::vector<std::string>& cont);
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
	std::string nickname = "ANON";
	bool movable{};
	NetworkPlayer(sf::TcpSocket& co) : Player(300), socket(co), ctrl() {
		clock.restart();
		movable = true;
	};
	std::string getNickname() {
		return nickname;
	}
	void setNickname(std::string newNickname) {
		nickname = newNickname;
	}
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
	std::string getLeaderboardData();
	bool getConnecting();	
	bool isWorking();	
	void connect();
	void cancelConnect();	
	void disconnect();	
	void test();	
	void awaitStart(std::atomic<State>& s);	
	void join(std::atomic<State>& s, char* nickname);
	void start();
	sf::TcpSocket& getSocket();
};

bool compareScore(const playerScore& p1, const playerScore& p2);

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
	bool endOnce = false;
	void endGame() {
		sf::Packet message;
		std::stringstream stream;
		message << "SCORE";
		for (auto& p : players) {
			stream << p->score.getScore() << " ";
		}
		if (!endOnce) {
			std::vector<playerScore> scores;
			for (auto& p : players) {
				scores.push_back({ p->getNickname(), p->score.getScore() });
			}
			addToScoreboard(scores);
			endOnce = true;
		}
		std::string temp = stream.str();
		temp.pop_back();
		message << temp;
		for (auto& p : players) {
			p->socket.send(message);
		}
	}
public:
	std::string getLeaderboardData() {
		std::ifstream file("Scoreboard.txt");
		std::stringstream content;
		std::vector<std::string> data;
		content << file.rdbuf();
		return content.str();
	}
	void deserializeTo(std::string input, std::vector<playerScore>& vec) {
		std::vector<std::string> tempVec;
		splitTo(input, ' ', tempVec);
		for (std::string& playerString : tempVec) {
			std::pair<std::string, std::string> playerPair = splitOnceBy(playerString, '|');
			std::cout << playerPair.second << std::endl;
			playerPair.second[0] = '0'; // override |
			int score = std::atoi(playerPair.second.c_str());
			vec.push_back({ playerPair.first, score });
		}
	}
	void addToScoreboard(std::vector<playerScore> newScores) {
		std::vector<playerScore> data;
		deserializeTo(getLeaderboardData(), data);
		for (auto x : data) {
			std::cout << x.first << "  " << x.second << std::endl;
		}
		for (auto x : newScores) {
			data.push_back(x);
		}
		std::sort(data.begin(), data.end(), compareScore);
		std::cout << "---" << std::endl;
		for (auto x : data) {
			std::cout << x.first << "  " << x.second << std::endl;
		}
		std::stringstream toBeSaved;
		for (int i{}; i < std::min(10, (int)data.size()); i++) {
			toBeSaved << data[i].first << '|' << data[i].second << " ";
		}
		std::string result = toBeSaved.str();
		std::ofstream file{"Scoreboard.txt"};
		result.pop_back(); // get rid of last space
		file << result;
	}
	void start();
	bool isVictoryConditionMet();
	std::string serializePlayerData();
	void handleStart(sf::Packet& p, sf::TcpSocket& s);
	void startPathsIf(bool conditions);
	void handleUpdate(sf::Packet& incomingMessage, sf::TcpSocket& socket);
	void handleScoreboard(sf::TcpSocket& s);
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
