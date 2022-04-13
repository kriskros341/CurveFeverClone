#include <SFML/Graphics.hpp>
#include <iostream>
#include <cmath>
#include <csignal>
#include <map>

#define PI std::acos(0) * 2
sf::Vector2u screenSize(800, 800);
using std::cout; using std::endl;

/*
	A linked list that holds pointers to all the lines player creates
*/
struct Node {
	sf::VertexArray* value;
	Node* next = 0;
	Node(sf::VertexArray* v) : value(v) {};
	~Node() {
		cout << "deleted";
	}
};

struct LinkedList {
	Node* head = 0;
	int length = 0;
	// add item to back of the list
	void push_back(sf::VertexArray* v) {
		Node* newVal = new Node(v);
		length++;
		if (!head)
			head = newVal;
		else {
			Node* temp = head;
			while (temp->next) {
				temp = temp->next;
			}
			temp->next = newVal;
		}
	}
	// select value of node by index
	sf::VertexArray* at(int index) {
		if (!head) {
			std::cout << "The list is empty" << std::endl;
			raise(SIGILL);
		}
		if (index > length) {
			std::cout << "No such item in list" << std::endl;
			raise(SIGILL);
		}
		int n = 0;
		Node* temp = head;
		while (n != index) {
			if (temp->next) {
				temp = temp->next;
			}
			n++;
		}
		if (temp) {
			return temp->value;
		}
		return head->value;
	};
	// select value of node by index
	sf::VertexArray* operator [](int index) {
		return at(index);
	}
	// empty the list in memory safe way
	void clear() {
		length = 0;
		if (head == 0) {
			return;
		}
		Node* temp1 = head;
		Node* temp2 = 0;
		while (temp1->next) {
			temp2 = temp1;
			temp1 = temp2->next;
			delete temp2;
		}
		delete temp1;
		//delete head;
		head = 0;
		//delete temp1;
	}
	~LinkedList() {
		clear();
	};
};

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

// 
class Player {
	bool placesPath = true;
	int lineIndex = 0;
	LinkedList* linesArray;
	
	sf::Vector2f position;
	sf::Vector2f starting;
	int size;

public:
	friend class MyRenderWindow;
	Player(sf::Vector2f p, LinkedList& la, int s = 2) : position(p), starting(p) {
		size = s;
		placesPath = true;
		linesArray = &la;
		
		initiateLine();
		//restart();
	};

	void initiateLine() {
		lineIndex++;
		cout << lineIndex << endl;
		sf::VertexArray* currentLine = new sf::VertexArray;
		currentLine->setPrimitiveType(sf::TrianglesStrip);
		linesArray->push_back(currentLine);
	}

	void createPathFrom(sf::Vector2f previous) {
		// TODO: Apply inverse matrix to the board so that it doesnt have to be -
	}

	// Get current position on the map
	sf::Vector2f getPosition() {
		return position;
	}
	// Get starting position
	sf::Vector2f getStarting() {
		return starting;
	}
	void applyDisplacement(float distance, float angle) {
		position += {distance* std::cos(angle), distance* std::sin(angle)};
	}

	// Move player by vector
	void applyDisplacement(Vector v) {
		position += v.getDisplacement();
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
		position = starting;
		placesPath = true;
		linesArray->clear();
		lineIndex = 0;
		initiateLine();
	}
	// Get size of player
	int getSize() {
		return size;
	}
	// Set size of player
	void setSize(int n) {
		size = n;
	}
	int getLineIndex() {
		return lineIndex;
	}
};

class MyRenderWindow : public sf::RenderWindow {
public:
	MyRenderWindow(sf::VideoMode v, std::string title) : sf::RenderWindow(v, title) {}
	void draw(Player p) {
		sf::CircleShape playerDot;
		playerDot = sf::CircleShape(p.size, 10);
		playerDot.setOrigin(p.size, p.size);
		playerDot.setPosition(p.getPosition());
		sf::RenderWindow::draw(playerDot);
		LinkedList* l = p.linesArray;
		for (int i{}; i < p.lineIndex; i++) {
			sf::RenderWindow::draw(*l->at(i));
		}

		//Vector forShow(40, angleFromPreviousPoint + PI / 2);
		//sf::Vertex show[] = { p.position, p.position + forShow.getDisplacement() };
		//sf::RenderWindow::draw(show, 2, sf::LinesStrip);
	}
};

/*
	TODO:
	move angle to player
	find a way to move this fucking linkedlist within player in memory safe way ffs
	texture/color of lines
	initially local multiplayer, then game server
	collision detection
*/

int main() {
	// initiate window and globally used values
	MyRenderWindow window(sf::VideoMode(screenSize.x, screenSize.y), "SFML");
	window.setFramerateLimit(60);
	float currentTick{};
	float currentAngle{};
	
	// initiate game clock
	sf::Clock clock;
	float elapsed = 0;

	// initiate player and all that has to do with them
	// doing linesArray stuff directly within player object breaks everything ; . ;
	LinkedList linesArray;
	Player player({400, 400}, linesArray);
	sf::Vector2f previous = player.getPosition();
	sf::Vector2f currentPosition;

	//initiate keymap (used to negate keyboard input lag)
	std::map<std::string, bool> keymap;
	
	while (window.isOpen()) {
		clock.restart();
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
				if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
					keymap["A"] = true;
				};
				if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
					keymap["D"] = true;
				};
				if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space)) {
					keymap["SPACE"] = true;
				}
				// restart the game
				if (sf::Keyboard::isKeyPressed(sf::Keyboard::LControl) && sf::Keyboard::isKeyPressed(sf::Keyboard::R)) {
					player.restart();
					currentAngle = 0;
					currentTick = 0;
					previous = player.getStarting();
				}
				break;
			};
			case(sf::Event::KeyReleased): {
				if (event.key.code == sf::Keyboard::Space) {
					// when space back up create new line
					player.initiateLine();
					keymap["SPACE"] = false;
				}
				if (event.key.code == sf::Keyboard::A) {
					keymap["A"] = false;
				}
				if (event.key.code == sf::Keyboard::D) {
					keymap["D"] = false;
				}
				break;
			}
			}
		}
		elapsed = clock.getElapsedTime().asMicroseconds();
		
		// handle keys after they are set for clarity sake
		if (keymap["A"]) {
			currentAngle -= 0.1 + (0.0001 * elapsed);
		}
		if (keymap["D"]) {
			currentAngle += 0.1 + (0.0001 * elapsed);
		}
		player.setPlacesPath(!keymap["SPACE"]);

		// Move player
		Vector displacement(5, currentAngle);
		player.applyDisplacement(displacement);

		// Do not touch without a commit. Memory safe, but very fragile;
		currentPosition = player.getPosition();
		float angleFromPreviousPoint = - atan2f(previous.x - currentPosition.x, previous.y - currentPosition.y);
		int lineIndex = player.getLineIndex();
		if (player.getPlacesPath()) {
			Vector c1(4, angleFromPreviousPoint + 7 * PI / 6);
			Vector c2(4, angleFromPreviousPoint - 1 * PI / 6);
			Vector c3(4, angleFromPreviousPoint + 7 * PI / 6);
			Vector c4(4, angleFromPreviousPoint - 1 * PI / 6);
			linesArray[lineIndex]->append(sf::Vertex(previous + c1.getDisplacement()));
			linesArray[lineIndex]->append(sf::Vertex(previous + c2.getDisplacement()));
			linesArray[lineIndex]->append(sf::Vertex(currentPosition + c3.getDisplacement()));
			linesArray[lineIndex]->append(sf::Vertex(currentPosition + c2.getDisplacement()));
		}

		previous = currentPosition;

		//draw stuff
		window.clear();
		window.draw(player);
		window.display();
		// increment game tick by a value modified by time between frames
		// this is supposed to make gameplay independent of frames per second and ping in the fututre
		currentTick = currentTick + 5 * elapsed;
	}
	return 0;
}
