#include "Window.h"

/// Function for drawing player, his position and size
void MyRenderWindow::draw(Player& player) {
	sf::CircleShape playerDot;
	float p = player.getSize();
	playerDot = sf::CircleShape(p, 10); //size jest w klasie Player, w ramach kt�rej jest tworzony obiekt z referencj� "p", mu s� przypisywane rzeczy
	playerDot.setOrigin(p, p);
	playerDot.setPosition(player.getPosition());
	sf::RenderWindow::draw(playerDot);
	for (auto l = player.linesArray.begin(); l != player.linesArray.end(); ++l) {
		sf::RenderWindow::draw(**l);
	}
}
/// Function for drawing background
void MyRenderWindow::draw(BackgroundImage& bcg) { //w ramach klasy BackgroundImage tworzony jest obiekt bcg, 
	sf::RenderWindow::draw(bcg.sprit); //on jest referencj�!
}
/// Function for drawing shape of player
void MyRenderWindow::draw(sf::Shape& s) {
	sf::RenderWindow::draw(s);
}
/// Fuction for drawing vertex array, points that form lines
void MyRenderWindow::draw(sf::VertexArray& v) {
	sf::RenderWindow::draw(v);
}
