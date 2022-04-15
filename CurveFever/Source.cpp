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
	MyRenderWindow(sf::VideoMode v, std::string title, sf::ContextSettings c) : sf::RenderWindow(v, title, sf::Style::Close, c) {}
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
	}
		void draw(sf::Shape& s) {
			sf::RenderWindow::draw(s);
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

int** getMatrixTransform(float angle) {
	float sin = std::sin(angle);
	float cos = std::cos(angle);
	int** matrix = new int* [2];
	for (int i{}; i < 2; i++) {
		matrix[i] = new int[2];
	}
	matrix[0][0] = cos;
	matrix[0][1] = -sin;
	matrix[1][0] = sin;
	matrix[1][1] = cos;
	return matrix;
}


sf::Vector2f mmul (int** m, sf::Vector2f v) {
	return {v.x * m[0][0] + v.y * m[0][1], v.x * m[1][0] + v.y * m[1][1]};
}


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
	LinkedList linesArray;
	Player player({400, 400}, linesArray); //starting position, lines
	sf::Vector2f previous = player.getPosition();
	sf::Vector2f currentPosition;

	//initiate keymap (used to negate keyboard input lag)
	std::map<std::string, bool> keymap;

	const int buffer = 5;
	const int screenWidth = screenSize.x;
	const int screenHeight = screenSize.y;
	
	Tilemap tilemap(screenWidth + 2 * buffer, screenHeight + 2 * buffer);



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
		elapsed = clock.restart().asMicroseconds();

		// handle keys after they are set for clarity sake
		if (keymap["A"]) {
			currentAngle -= (0.00001 * elapsed);
		}
		if (keymap["D"]) {
			currentAngle += (0.00001 * elapsed);
		}
		player.setPlacesPath(!keymap["SPACE"]);

		// Move player
		Vector displacement(5, currentAngle);
		player.applyDisplacement(displacement);

		// Do not touch without a commit. Memory safe, but very fragile;
		currentPosition = player.getPosition();
		float angleFromPreviousPoint = -atan2f(previous.x - currentPosition.x, previous.y - currentPosition.y);
		int lineIndex = player.getLineIndex();
		std::cout << currentPosition.x << std::endl;

		//check for collision with screen border
		if (currentPosition.x + buffer > screenHeight ||
			currentPosition.x - buffer < 0 ||
			currentPosition.y + buffer > screenWidth ||
			currentPosition.y - buffer < 0
			) 
		{
			player.restart();
			currentAngle = 0;
			currentTick = 0;
			tilemap.init();
			previous = player.getStarting();
			continue;
		}

		//check for collision with path
		sf::Vector2i currentCoords(currentPosition);
		sf::RectangleShape s({ (float)player.getSize() * 2.0f, 10 });
		s.setOrigin({ player.getSize() / 2.0f }, 5);
		s.setPosition({ (float)currentCoords.x, 0 });

		bool toBeTerminated = false;
		for (int i = currentCoords.x - player.getSize() * 4; i < currentCoords.x + player.getSize() * 4; i++) {
			for (int j = currentCoords.y - player.getSize() * 4; j < currentCoords.y + player.getSize() * 4; j++) {
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
			player.restart();
			currentAngle = 0;
			currentTick = 0;
			tilemap.init();
			previous = player.getStarting();
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


			/*
				Now that I know how triangle strip works...
			*/
			//Vector c1(4, angleFromPreviousPoint + 7 * PI / 6);
			//Vector c2(4, angleFromPreviousPoint - 1 * PI / 6);
			//sf::Vertex v1 = sf::Vertex(previous + c1.getDisplacement());
			//sf::Vertex v2 = sf::Vertex(previous + c2.getDisplacement());
			
			//linesArray[lineIndex]->append(v1);
			//linesArray[lineIndex]->append(v2);
			linesArray[lineIndex]->append(v3);
			linesArray[lineIndex]->append(v4);
			
			int x1 = (int)v3.position.x;
			int x2 = (int)v4.position.x;
			int y1 = (int)v3.position.y;
			int y2 = (int)v4.position.y;

			int maxx = x1 > x2 ? x1 : x2;
			int minx = x1 < x2 ? x1 : x2;

			int maxy = y1 > y2 ? y1 : y2;
			int miny = y1 < y2 ? y1 : y2;
			
			//for (int i = minx; i <= maxx; i++) {
			//	for (int j = miny; j <= maxy; j++) {
			//		tilemap.setAt(i, j, 1);
			//	}
			//}


			
			
			tilemap.setAt((int)v3.position.x, (int)v3.position.y, 1);
			tilemap.setAt((int)v4.position.x, (int)v4.position.y, 1);

			

		}

		previous = currentPosition;

		//draw stuff
		window.clear();
		window.draw(s);
		window.draw(player);


		/*
		
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
		*/
		
		
		window.display();
		// increment game tick by a value modified by time between frames
		// this is supposed to make gameplay independent of frames per second and ping in the fututre
		currentTick = currentTick + 5 * elapsed;
	}
	return 0;
}
