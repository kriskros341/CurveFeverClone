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


template <typename T>
struct Node {
	T value;
	Node* next = 0;
	Node(T v) : value(v) {};
	~Node() {
		cout << "deleted";
	}
};


template <typename T>
struct LinkedListDynamic {
	Node<T*>* head = 0;

	//NOT IMPL
	Node<T*>* tail = 0;
	
	int length = 0;
	// add item to back of the list
	void push_back(T* v) {
		Node<T*>* newVal = new Node<T*>(v);
		length++;
		if (!head)
			head = newVal;
		else {
			Node<T*>* temp = head;
			while (temp->next) {
				temp = temp->next;
			}
			temp->next = newVal;
		}
	}
	// select value of node by index
	T* at(int index) {
		if (!head) {
			std::cout << "The list is empty" << std::endl;
		}
		if (index > length) {
			std::cout << "No such item in list" << std::endl;
		}
		int n = 0;
		Node<T*>* temp = head;
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
	T* operator [](int index) {
		return at(index);
	}

	// empty the list in memory safe way
	void clear() {
		length = 0;
		if (head == 0) {
			return;
		}
		Node<T*>* temp1 = head;
		Node<T*>* temp2 = 0;
		while (temp1->next) {
			temp2 = temp1;
			temp1 = temp2->next;
			delete temp2->value;
			delete temp2;
		}
		delete temp1->value;
		delete temp1;
		//delete head;
		head = 0;
		//delete temp1;
	}
	~LinkedListDynamic() {
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
	LinkedListDynamic<sf::VertexArray>* linesArray;
	
	sf::Vector2f position;
	sf::Vector2f starting;
	int size;

public:
	friend class MyRenderWindow;
	Player(sf::Vector2f p, LinkedListDynamic<sf::VertexArray>& la, int s = 2) : position(p), starting(p) {
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
	MyRenderWindow(sf::VideoMode v, std::string title, sf::ContextSettings c) : sf::RenderWindow(v, title, sf::Style::Close, c) {}
	void draw(Player p) {
		sf::CircleShape playerDot;
		playerDot = sf::CircleShape(p.size, 10);
		playerDot.setOrigin(p.size, p.size);
		playerDot.setPosition(p.getPosition());
		sf::RenderWindow::draw(playerDot);
		LinkedListDynamic<sf::VertexArray>* l = p.linesArray;
		for (int i{}; i < p.lineIndex; i++) {
			sf::RenderWindow::draw(*l->at(i));
		}
	}
		void draw(sf::Shape& s) {
			sf::RenderWindow::draw(s);
		}
		void draw(sf::VertexArray& v) {
			sf::RenderWindow::draw(v);
		}

		//Vector forShow(40, angleFromPreviousPoint + PI / 2);
		//sf::Vertex show[] = { p.position, p.position + forShow.getDisplacement() };
		//sf::RenderWindow::draw(show, 2, sf::LinesStrip);
};

/*
	TODO:
	move angle to player
	find a way to move this fucking linkedlist within player in memory safe way ffs
	texture/color of lines
	initially local multiplayer, then game server
	collision detection
*/

class Tilemap {
	int** tileMap = 0;
	int width, height;
public:
	Tilemap(int w, int h) {
		width = w; height = h;
		init();
	}
	void init() {
		if (tileMap) {
			for (int i{}; i < width; i++) {
				delete[] tileMap[i];
			}
			delete[] tileMap;
		}
		int** outer = new int* [width];
		for (int i{}; i < width; i++) {
			outer[i] = new int[height]{};
		}
		tileMap = outer;
	}
	void setAt(int i, int j, int val) {
		tileMap[i][j] = val;
	}
	int getAt(int i, int j) {
		return tileMap[i][j];
	}
};

float** getMatrixRotate(float angle) {
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

sf::Vector2f midpoint(sf::Vector2f v1, sf::Vector2f v2) {
	return (v1 + v2) / 2.0f;
}

sf::Vector2f mmul (float** m, sf::Vector2f v) {
	return {v.x * m[0][0] + v.y * m[0][1], v.x * m[1][0] + v.y * m[1][1]};
}

float** transpose(float** m) {
	float** n = new float*[2];
	n[0] = new float[2]{ m[0][0], m[1][0] };
	n[1] = new float[2]{ m[0][1], m[1][1] };
	return n;
}


float distance(sf::Vector2f a, sf::Vector2f b) {
	return std::sqrt(std::pow(a.x - b.x, 2) + std::pow(a.y - b.y, 2));
}

void printv(sf::Vector2f v) {
	std::cout << v.x << " " << v.y << std::endl;
}

// On linked list vs pointer
// https://fylux.github.io/2017/06/29/list_vs_vector/
// Linked lists much worse for small objects
// find better on vector, insert delete on list for front/back

//switch both list of vertices and list of POI to vectors (sf::Vertex and std::pair<float, float>
int main() {
	// initiate window and globally used values
	sf::ContextSettings settings;
	settings.antialiasingLevel = 4.0;
	MyRenderWindow window(sf::VideoMode(screenSize.x, screenSize.y), "SFML", settings);
	window.setFramerateLimit(60);
	float currentTick{};
	float currentAngle{};
	
	// initiate game clock
	sf::Clock clock;
	float elapsed = 0;

	// initiate player and all that has to do with them
	// doing linesArray stuff directly within player object breaks everything ; . ;
	LinkedListDynamic<sf::VertexArray> linesArray;
	Player player({400, 400}, linesArray); //starting position, lines
	sf::Vector2f previous = player.getPosition();
	sf::Vector2f currentPosition;

	//initiate keymap (used to negate keyboard input lag)
	std::map<std::string, bool> keymap;

	const int buffer = 5;
	const int screenWidth = screenSize.x;
	const int screenHeight = screenSize.y;
	
	Tilemap tilemap(screenWidth + 2 * buffer, screenHeight + 2 * buffer);
	sf::VertexArray* arrow = new sf::VertexArray;
	player.setSize(10);

	std::vector<sf::Vector2f> POIq;

	//Make it all into a  game object
	auto restart = [&]() {
		player.restart();
		currentAngle = 0;
		currentTick = 0;
		tilemap.init();
		POIq.clear();
		previous = player.getStarting();
	};

	float angleFromPreviousPoint{};

	auto debug = [&]() {
		arrow->clear();
		sf::Vertex v1(currentPosition);
		float** g = getMatrixRotate(angleFromPreviousPoint - PI/2);
		sf::Vertex v2(currentPosition + mmul(g, sf::Vector2f(40, 0)));
		arrow->append(v1);
		arrow->append(v2);
		delete[] g[0];
		delete[] g[1];
		delete[] g;

		sf::CircleShape distanceIndicator(player.getSize() * 2, 5);
		distanceIndicator.setFillColor(sf::Color::Magenta);
		distanceIndicator.setOrigin(player.getSize() * 2, player.getSize() * 2);
		distanceIndicator.setPosition(currentPosition);
		window.draw(distanceIndicator);
		for (int i{}; i < screenWidth; i++) {
			for (int j{}; j < screenHeight; j++) {
				if (tilemap.getAt(i, j) == 1) {
					sf::CircleShape s(2, 4);
					s.setFillColor(sf::Color::Red);
					s.setPosition(i, j);
					window.draw(s);
				}
			}
		}
	};

	bool doDebug = false;

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
				if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
					keymap["A"] = true;
				};
				if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
					keymap["D"] = true;
				};
				if (sf::Keyboard::isKeyPressed(sf::Keyboard::B)) {
					keymap["B"] = true;
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
				if (event.key.code == sf::Keyboard::B) {
					keymap["B"] = false;
				}
				break;
			}
			}
		}
		elapsed = clock.restart().asMicroseconds();

		// handle keys after they are set for clarity sake
		if (keymap["A"]) {
			currentAngle -= (0.00001 * elapsed);
		}
		if (keymap["D"]) {
			currentAngle += (0.00001 * elapsed);
		}
		doDebug = keymap["B"];
		player.setPlacesPath(!keymap["SPACE"]);

		// Move player
		Vector displacement(5, currentAngle);
		player.applyDisplacement(displacement);

		// Do not touch without a commit. Memory safe, but very fragile;
		currentPosition = player.getPosition();
		angleFromPreviousPoint = -atan2f(previous.x - currentPosition.x, previous.y - currentPosition.y);
		int lineIndex = player.getLineIndex();

		//check for collision with screen border
		if (currentPosition.x + buffer > screenHeight ||
			currentPosition.x - buffer < 0 ||
			currentPosition.y + buffer > screenWidth ||
			currentPosition.y - buffer < 0
			)
		{
			restart();
			continue;
		}

		//check for collision with path
		sf::Vector2i currentCoords(currentPosition);

		bool toBeTerminated = false;
		for (int i = currentCoords.x - player.getSize(); i < currentCoords.x + player.getSize(); i++) {
			for (int j = currentCoords.y - player.getSize(); j < currentCoords.y + player.getSize(); j++) {
				if (tilemap.getAt(i, j) == 1) {
					//check distance from center is lower than player radius
					double distance = std::sqrt(std::pow(i - currentCoords.x, 2) + std::pow(j - currentCoords.y, 2));
					if (distance < player.getSize()) {
						std::cout << "JEBAC JAK PLATFORMA NIE JEBALA" << std::endl;
						toBeTerminated = true;
						break;
					}

				}
			}
			if (toBeTerminated)
				break;
		}
		if (toBeTerminated) {
			restart();
			continue;
		}

		if (player.getPlacesPath()) {
			Vector c1(player.getSize(), angleFromPreviousPoint + 7 * PI / 6);
			Vector c2(player.getSize(), angleFromPreviousPoint - 1 * PI / 6);
			Vector c3(player.getSize(), angleFromPreviousPoint + 7 * PI / 6);
			Vector c4(player.getSize(), angleFromPreviousPoint - 1 * PI / 6);

			sf::Vertex v1 = sf::Vertex(previous + c1.getDisplacement());
			sf::Vertex v2 = sf::Vertex(previous + c2.getDisplacement());
			sf::Vertex v3 = sf::Vertex(currentPosition + c3.getDisplacement());
			sf::Vertex v4 = sf::Vertex(currentPosition + c4.getDisplacement());
			linesArray[lineIndex]->append(v3);
			linesArray[lineIndex]->append(v4);


			// I think I need full mesh
			//POIq.push_back(v1.position);
			//POIq.push_back(v2.position);
			//POIq.push_back(v3.position);
			//POIq.push_back(v4.position);
			POIq.push_back(midpoint(v1.position, v4.position));
			/* add these to queue and add to tilemap only after player leaves the radius */

			for (int i{}; i < POIq.size(); i++) {
				if (distance(POIq[i], currentPosition) > player.getSize() * 2) {
					sf::Vector2f p = *POIq.erase(POIq.begin() + i);
					tilemap.setAt(round(p.x), round(p.y), 1);
				}
			}
		}

		previous = currentPosition;

		//draw stuff
		window.clear();
		window.draw(*arrow);
		window.draw(player);

		// TileMap debug
		if(doDebug) debug();
		
		
		
		window.display();
		// increment game tick by a value modified by time between frames
		// this is supposed to make gameplay independent of frames per second and ping in the fututre
		currentTick = currentTick + 5 * elapsed;
	}
	delete[] arrow;
	return 0;
}
