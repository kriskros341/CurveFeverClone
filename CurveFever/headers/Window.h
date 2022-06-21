#pragma once
#include "Player.h"
#include <SFML/Graphics.hpp>

/// Class for managing background of the main screen
class BackgroundImage {
public:
	std::string pngname;
	sf::Vector2u scrres;
	sf::Texture imag;
	sf::Sprite sprit;

	BackgroundImage(std::string pngname = "Texture/gg2.png") {
		imag.loadFromFile(pngname);
		scrres = imag.getSize();
		sprit.setTexture(imag);
		sprit.setScale(sf::Vector2f(screenSize.x / (float)scrres.x, screenSize.x / (float)scrres.y));
	};
	~BackgroundImage() {};
};

/// Class for initializing window
class MyRenderWindow : public sf::RenderWindow {
public:
	sf::Clock guiClock;
	MyRenderWindow(sf::VideoMode v, std::string title, sf::ContextSettings c) : sf::RenderWindow(v, title, sf::Style::Close, c)
	{
		guiClock = sf::Clock();
	}
	void draw(Player& player);
	void draw(sf::Text& text) {
		sf::RenderWindow::draw(text);
	};
	void draw(BackgroundImage& bcg);
	void draw(sf::Shape& s);
	void draw(sf::VertexArray& v);
};
