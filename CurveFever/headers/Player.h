#pragma once
#include <SFML/Graphics.hpp>
#include <iostream>
#define PI std::acos(0) * 2
const sf::Vector2u screenSize(1000, 1000);

sf::Vector2f midpoint(sf::Vector2f v1, sf::Vector2f v2);

class Vector {
	float length{};
	float angle{};
public:
	Vector(float l, float a) : length(l), angle(a) {};
	sf::Vector2f getDisplacement();
};

float distance(sf::Vector2f a, sf::Vector2f b);
class PositionManager {
	float angle{};
	sf::Vector2f current;
	sf::Vector2f previous;
	sf::Vector2f starting;
	float startingDistance{};
	friend class Player;
public:
	PositionManager(sf::Vector2f p) : current(p), starting(p), previous(p) {}
	void restart();
	void pickNewPosition(float radius) {
		angle = rand() % 360;
		sf::Vector2f r = { radius, 0 };
		sf::Vector2f secr = { screenSize.x / 2 + (r.x * cos(angle) - r.y * sin(angle)),screenSize.y / 2 + (r.x * sin(angle) + r.y * cos(angle)) };
		current = secr, starting = secr, previous = secr;
	}
	PositionManager(float radius = 300) : startingDistance(radius) {
		//std::cout << std::cos(angle) << ", " << std::sin(angle) << ", radians: " << angle * PI / 180 << std::endl;
		pickNewPosition(startingDistance);
		
	}
	float getDistanceFromPrevious();
	float getAngleFromPrevious();
	void setAngle(float a);
	float getAngle();
	void changeAngle(float a);
	void applyDisplacement(float distance, float angle);
	void setPosition(const sf::Vector2f& n);
	void applyDisplacement(Vector& v);
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
	std::vector<sf::VertexArray*> linesArray;
	std::vector<std::pair<float, float>> collisionPointMap;
	std::vector<std::pair<float, float>> collisionPointQueue;
	LineManager() {
		linesArray.reserve(100);
		collisionPointMap.reserve(10000);
		collisionPointQueue.reserve(100);
	}
	void initiateLine();	
	void restart();
	int getLineIndex();	
	void setLineindex(int i);
	virtual ~LineManager() {
		collisionPointQueue.clear();
		collisionPointMap.clear();
		linesArray.clear();
	}
};

class ScoreManager {
	int startingScore = 0;
	std::vector<int> scoreBoard;
	int round = 0;
public:
	ScoreManager() {
		scoreBoard.push_back(startingScore);
	}
	void nextRound() {
		scoreBoard.push_back(startingScore);
		round += 1;
	}
	int getCurrentScore() {
		return scoreBoard[round];
	}
	int getScore() {
		int result{};
		for (auto x : scoreBoard) {
			result += x;
		}
		return result;
	}
	void addScore() {
		scoreBoard[round] += 1;
	
	}
	void restart() {
		scoreBoard.clear();
		scoreBoard.emplace_back(startingScore);
		round = 0;
	}
	void addScore(int add) {
		scoreBoard[round] += add;
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
	ScoreManager score;
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
	void restart(sf::Vector2f newPos) {
		restart();
		setPosition(newPos);
	}
	// restart game
	void restart() {
		placesPath = false;
		LineManager::restart();
		PositionManager::restart();
		score.restart();
		placesPath = true;
		linerestart.restart();
	}
	void setLineMode(LineModes newMode);
	int getId();	
	void moveTo(sf::Vector2f newp);
	void moveBy(float distance);
	// Get current position on the map
	sf::Vector2f getPosition();
	// Get starting position
	sf::Vector2f getStarting();
	// Set whether player places path
	void setPlacesPath(bool v);
	// Get whether player places path
	bool getPlacesPath();
	// Get size of player
	int getSize();	
	// Set size of player
	void setSize(int n);
	bool checkForCollision();	
	bool checkForCollision(Player& other);
	void updateCollisionQueue();
	int randRGBv0 = rand() % 255;
	int randRGBv1 = rand() % 255;
	int randRGBv2 = rand() % 255;

	void setVisualPath();
	void setCollisionPath();
	void setPath();
	~Player() {}
};
