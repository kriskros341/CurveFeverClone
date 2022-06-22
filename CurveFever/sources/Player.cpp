#include "Player.h"

sf::Vector2f midpoint(sf::Vector2f v1, sf::Vector2f v2) {
	return (v1 + v2) / 2.0f;
}
sf::Vector2f Vector::getDisplacement() {
	return { length * std::cos(angle), length * std::sin(angle) };
}

float distance(sf::Vector2f a, sf::Vector2f b) {
	auto vec = b - a;
	return std::sqrt(vec.x * vec.x + vec.y * vec.y);
};
/// Function that restarts position of players after collision
void PositionManager::restart() {
	angle = rand() % 360;
	current = starting;
	previous = starting;
}
float PositionManager::getDistanceFromPrevious() {
	return distance(current, previous);
}

float PositionManager::getAngleFromPrevious() {
	return -atan2f(previous.x - current.x, previous.y - current.y) + PI / 2;
}
/// Setting angle
void PositionManager::setAngle(float a) {
	angle = a;
}
float PositionManager::getAngle() {
	return angle;
}
/// Changing angle
void PositionManager::changeAngle(float a) {
	angle += a;
}
/// Function that applies change of position, including angles
void PositionManager::applyDisplacement(float distance, float angle) {
	current += {distance * std::cos(angle), distance * std::sin(angle)};
}
/// Setting position
void PositionManager::setPosition(const sf::Vector2f& n) {
	previous = current;
	current = n;
}
/// Applying displacement
void PositionManager::applyDisplacement(Vector& v) {
	previous = current;
	current = current + v.getDisplacement();
}
/// Function that initiate line, incrementing array of vertices
void LineManager::initiateLine() {
	lineIndex++;
	sf::VertexArray* currentLine = new sf::VertexArray;
	currentLine->setPrimitiveType(sf::TrianglesStrip);
	linesArray.push_back(currentLine);
}
/// Function that clears arrays, restarting them
void LineManager::restart() {
	linesArray.clear();
	collisionPointMap.clear();
	collisionPointQueue.clear();
	lineIndex = startingLineIndex;
	initiateLine();
	
}
int LineManager::getLineIndex() {
	return lineIndex;
}
/// Incrementing line index
void LineManager::setLineindex(int i) {
	lineIndex = i;
}
void Player::setLineMode(LineModes newMode) {
	lineMode = newMode;
}
int Player::getId() {
	return id;
}
/// Moving player to different position through its movement
void Player::moveTo(sf::Vector2f newp) {
	if (placesPath) {
		setPath();
		updateCollisionQueue();
	}
	setPosition(newp);
}
/// Function that randomly delays placing of path
void Player::chooseWhetherToPlacePathOrNot() {
	sf::Time time1;
	time1 = clock1.getElapsedTime();
	if (time1.asMilliseconds() % 15 == 0) {
		if (!placesPath) {
			initiateLine();
		}
		placesPath = !placesPath;
	}
}
/// Function that move line by perticular distance and adds score
void Player::moveBy(float distance) {
	chooseWhetherToPlacePathOrNot();
	if (placesPath) {
		score.addScore();
		setPath();
		updateCollisionQueue();
	}
	Vector displacement(distance, getAngle());
	applyDisplacement(displacement);
}

/// Get current position on the map
sf::Vector2f Player::getPosition() {
	return current;
}
/// Get starting position
sf::Vector2f Player::getStarting() {
	return starting;
}
/// Set whether player places path
void Player::setPlacesPath(bool v) {
	if (placesPath == false && v == true) {
		initiateLine();
	}
	placesPath = v;
};
/// Get whether player places path
bool Player::getPlacesPath() {
	return placesPath;
};
/// Get size of player
int Player::getSize() {
	return size;
}
/// Set size of player
void Player::setSize(int n) {
	size = n;
}

bool Player::checkForCollision() {
	/// check for collision with screen border, then check for collision with path
	if (current.x > screenSize.x ||
		current.x < 0 ||
		current.y > screenSize.y ||
		current.y < 0)
	{
		return true;
	}
	for (std::pair<float, float> point : collisionPointMap) {
		if (distance(current, {point.first, point.second}) < size) {
			return true;
		}
	}
	return false;
}


bool Player::checkForCollision(Player& other) {
	/// check for collision with screen border, then check for collision with path
	if (current.x > screenSize.x ||
		current.x < 0 ||
		current.y > screenSize.y ||
		current.y < 0)
	{
		return true;
	}
	for (std::pair<float, float> point : other.collisionPointMap) {
		if (abs(point.first - getPosition().x) < size * 2) { // calculate only if near on x axis
			if (distance(current, {point.first, point.second}) < size) {
				return true;
			}
		}
	}
	return false;
}

/// Function that iterates and checks whether collision occured
void Player::updateCollisionQueue() {
	std::vector<std::pair<float, float>>::iterator queuedPoint = collisionPointQueue.begin();
	for (; queuedPoint != collisionPointQueue.end();) {
		if (distance(current, { queuedPoint->first, queuedPoint->second }) > size * 2) {
			collisionPointMap.push_back(*queuedPoint);
			///rescure p address;
			queuedPoint = collisionPointQueue.erase(queuedPoint);
		}
		else {
			///increment p address
			queuedPoint++;
		};
	}
}
/// Function that sets visual path to players
void Player::setVisualPath() {
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
/// Function that sets path used to initiate collision
void Player::setCollisionPath() {
	float angleFromPreviousPoint = getAngleFromPrevious();
	Vector c1(size, angleFromPreviousPoint + 1 * PI / 6);
	Vector c2(size, angleFromPreviousPoint - 1 * PI / 6);
	sf::Vertex v1 = sf::Vertex(previous + c1.getDisplacement(), sf::Color(255, 255, 0, 255));
	sf::Vertex v4 = sf::Vertex(current + c2.getDisplacement(), sf::Color(randRGBv0, randRGBv1, randRGBv2, 255));
	sf::Vector2f mid = midpoint(v1.position, v4.position);
	collisionPointQueue.push_back({ mid.x, mid.y });
}
/// Setting and initiating both visual and collision lines
void Player::setPath() {
	if (lineMode == LineModes::both || lineMode == LineModes::visual) {
		setVisualPath();
	}
	if (lineMode == LineModes::both || lineMode == LineModes::collision) {
		setCollisionPath();
	}
}
