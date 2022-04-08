#include <SFML/Graphics.hpp>
#include <iostream>
#include <cmath>
#include <csignal>
#include <map>

#define PI std::acos(0) * 2
sf::Vector2u screenSize(800, 800);

struct Node {
	sf::VertexArray* value;
	Node* next = 0;
	Node(sf::VertexArray* v) : value(v) {};
	~Node() {
		std::cout << "deleted";
	}
};

struct LinkedList {
	Node* head = 0;
	int length = 0;
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
		return temp->value;
	};
	sf::VertexArray* operator [](int index) {
		return at(index);
	}
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

class Vector {
	float length{};
	float angle{};
public:
	Vector(float l, float a) : length(l), angle(a) {};
	sf::Vector2f getDisplacement() {
		return { length * std::cos(angle), length * std::sin(angle) };
	}
};

class Player {
	sf::Vector2f position;
	bool placesPath;
	sf::Vector2f starting;
	int size;
public:
	Player(sf::Vector2f p) : position(p), starting(p) {
		placesPath = true;
		size = sqrt(2) / 2;
	};
	sf::Vector2f getPosition() {
		return position;
	}
	sf::Vector2f getStarting() {
		return starting;
	}
	void applyDisplacement(Vector v) {
		position += v.getDisplacement();
	}

	void setPlacesPath(bool v) {
		placesPath = v;
	};
	bool getPlacesPath() {
		return placesPath;
	};
	void restart() {
		position = starting;
		placesPath = true;
	}
	int getSize() {
		return size;
	}
	void setSize(int n) {
		size = n;
	}
};

bool checkForCollision(Player player, sf::VertexArray line) {
	sf::Vector2f position = player.getPosition();
	int radius = player.getSize();
	/*get nearest point to the player*/
	return true;
}

int main() {
	sf::RenderWindow window(sf::VideoMode(screenSize.x, screenSize.y), "SFML");
	//sf::Font font;
	//if (!font.loadFromFile("Arial.ttf"))
	//{
	//	return 0;
	//}
	Player p1({400, 400});
	float t{};
	float d{};
	float test = 2;
	
	sf::VertexArray* k = new sf::VertexArray;
	k->setPrimitiveType(sf::TrianglesStrip);

	LinkedList arr;
	arr.push_back(k);

	sf::Clock c;
	float elapsed = 0;
	window.setFramerateLimit(60);
	sf::Vector2f previous = p1.getPosition();
	sf::Vector2f currentPosition;
	sf::Vector2f testv = { 2, 2 };

	std::map<std::string, bool> keymap;
	int line_index = 1;
	while (window.isOpen()) {
		c.restart();
		sf::Event event;
		while (window.pollEvent(event)) {
			switch (event.type) {
			case(sf::Event::Closed): {
				window.close();
			}
			case(sf::Event::MouseButtonPressed): {
				while (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {}
				break;
			}
			case(sf::Event::KeyPressed): {
				if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
					keymap["A"] = true;
				};
				if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
					keymap["D"] = true;
				};
				if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space)) {
					p1.setPlacesPath(false);
				}
				if (sf::Keyboard::isKeyPressed(sf::Keyboard::LControl) && sf::Keyboard::isKeyPressed(sf::Keyboard::R)) {
					p1.restart();
					d = 0;
					t = 0;
					line_index = 1;
					arr.clear();
					sf::VertexArray* o = new sf::VertexArray;
					o->setPrimitiveType(sf::TrianglesStrip);
					arr.push_back(o);
					previous = p1.getStarting();
				}
				break;
			};
			case(sf::Event::KeyReleased): {
				if (event.key.code == sf::Keyboard::Space) {
					sf::VertexArray* o = new sf::VertexArray;
					o->setPrimitiveType(sf::TrianglesStrip);
					arr.push_back(o);
					line_index++;
					p1.setPlacesPath(true);
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
		if (keymap["A"]) {
			d -= 0.1 + (0.0001 * elapsed);
		}
		if (keymap["D"]) {
			d += 0.1 + (0.0001 * elapsed);
		}

		elapsed = c.getElapsedTime().asMicroseconds();
		Vector displacement(5, d);
		p1.applyDisplacement(displacement);
		sf::CircleShape l(2, 40);
		l.setOrigin(2, 2);
		
		currentPosition = p1.getPosition();
		float angleFromPreviousPoint = - atan2f(previous.x - currentPosition.x, previous.y - currentPosition.y);
		
		l.setPosition(p1.getPosition());
		Vector c1(4, angleFromPreviousPoint + 7 * PI / 6);
		Vector c2(4, angleFromPreviousPoint - 1 * PI / 6);
		Vector c3(4, angleFromPreviousPoint + 7 * PI / 6);
		Vector c4(4, angleFromPreviousPoint - 1 * PI / 6);

		Vector forShow(40, angleFromPreviousPoint + PI / 2);
		sf::Vertex show[] = { currentPosition, currentPosition + forShow.getDisplacement() };

		if (p1.getPlacesPath()) {
			arr[line_index]->append(sf::Vertex(previous + c1.getDisplacement()));
			arr[line_index]->append(sf::Vertex(previous + c2.getDisplacement()));
			arr[line_index]->append(sf::Vertex(currentPosition + c3.getDisplacement()));
			arr[line_index]->append(sf::Vertex(currentPosition + c4.getDisplacement()));
		}

		previous = currentPosition;
		window.clear();
		for (int i{}; i < line_index; i++) {
			window.draw(*arr[i]);
		}
		window.draw(l);
		window.draw(show, 2, sf::LinesStrip);
		window.display();
		t = t + 5 * elapsed;
	}
	return 0;
}
