#pragma once
#include "Network.h"

bool compareHosts(sf::TcpSocket& c, sf::TcpSocket& s) {
	return (
		c.getLocalPort() == c.getLocalPort() &&
		c.getRemoteAddress() == s.getRemoteAddress() &&
		c.getRemotePort() == s.getRemotePort());
}

void NetworkPlayer::processMovement(sf::Packet& p) {
	timeSinceLastUpdate = clock.restart();
	bool left, right, space;
	p >> left >> right >> space;
	changeAngle(-1 * left * 0.003 * timeSinceLastUpdate.asMilliseconds());
	changeAngle(1 * right * 0.003 * timeSinceLastUpdate.asMilliseconds());
	moveBy(0.05 * timeSinceLastUpdate.asMilliseconds());
}

bool networkClient::getConnected() {
	return isConnected;
}
bool networkClient::getConnecting() {
	return isConnecting;
}
bool networkClient::isWorking() {
	return isConnected || isConnecting;
}
void networkClient::connect() {
	isConnecting = true;
	socket.connect(ip, 5030);
	sf::Packet incomingMessage;
	socket.receive(incomingMessage);
	std::string data;
	incomingMessage >> data;
	if (data == w) {
		std::cout << "CONNECTED!" << std::endl;
		isConnected = true;
	}
	isConnecting = false;
};
void networkClient::cancelConnect() {
	socket.disconnect();
}
void networkClient::disconnect() {
	socket.disconnect();
	isConnected = false;
}
void networkClient::test() {
	sf::Packet p;
	std::string t = "TEST";
	p << t;
	socket.send(p);
}
void networkClient::awaitStart(std::atomic<State>& s) {
	sf::Packet p;
	std::string procedure;
	while (true) {
		socket.receive(p);
		p >> procedure;
		std::cout << "proc" << procedure << std::endl;
		if (procedure == "START") {
			s = State::multiplayer;
			break;
		}
	}
};
void networkClient::join(std::atomic<State>& s) {
	sf::Packet p;
	std::string t = "JOIN";
	p << t;
	socket.send(p);
	waiterThread.reset(new std::thread([&]() { awaitStart(s); }));
}
void networkClient::start() {
	sf::Packet p;
	std::string t = "START";
	p << t;
	socket.send(p);
}
sf::TcpSocket& networkClient::getSocket() {
	return socket;
}

void Server2ndTry::start() {
	clock.restart();
	std::thread t([&]() {acceptLoop();});
	std::thread y([&]() {recvLoop();});
	std::thread z([&]() {updateLoop();});
	while (true) {}
}
std::string Server2ndTry::serializePlayerData() {
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
void Server2ndTry::handleStart(sf::Packet& p, sf::TcpSocket& s) {
	sf::Packet ps;
	if (started) {
		return;
	}
	ps << "START";
	started = true;
	for (auto& player : players) {
		std::cout << "Sent start" << std::endl;
		player->clock.restart();
		player->socket.send(ps);
	}
	clock.restart();

}
void Server2ndTry::startPathsIf(bool conditions) {
	if (!pathsStarted && conditions) {
		std::cout << "PATH" << std::endl;
		for (auto& player : players) {
			player->setPlacesPath(true);
		}
		pathsStarted = true;
	}
}

void Server2ndTry::handleUpdate(sf::Packet& incomingMessage, sf::TcpSocket& socket) {
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

void Server2ndTry::updateLoop() {
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
void Server2ndTry::handleJoin(sf::Packet& IncomingMessage, sf::TcpSocket& socket) {
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

void Server2ndTry::parseRecieved(sf::Packet& incomingMessage, sf::TcpSocket& socket) {
	std::string procedure;
	incomingMessage >> procedure;
	if (procedure == "START")
		handleStart(incomingMessage, socket);
	if (procedure == "UPDATE")
		handleUpdate(incomingMessage, socket);
	if (procedure == "JOIN")
		handleJoin(incomingMessage, socket);
}

void Server2ndTry::acceptLoop() {
	while (true) {
		std::shared_ptr<sf::TcpSocket> client = std::make_shared<sf::TcpSocket>();
		sf::Packet outgoingMessage;
		outgoingMessage << "welcome";
		if (listener.listen(5030) != sf::Socket::Done) {
			std::cout << "Failed to listen on port" << std::endl;
		}
		if (listener.accept(*client) != sf::Socket::Done) {
			std::cout << "Failed to accept the client" << std::endl;
		}
		socketMutex.lock();
		sockets.push_back(client);
		client->send(outgoingMessage);
		selector.add(*client);
		std::cout << "Client accepted" << std::endl;
		socketMutex.unlock();
	}
}
void Server2ndTry::recvLoop() {
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
