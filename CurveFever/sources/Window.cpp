#include "Window.h"

void MyRenderWindow::draw(Player& player) {
	sf::CircleShape playerDot;
	float p = player.getSize();
	playerDot = sf::CircleShape(p, 10); //size jest w klasie Player, w ramach której jest tworzony obiekt z referencj¹ "p", mu s¹ przypisywane rzeczy
	playerDot.setOrigin(p, p);
	playerDot.setPosition(player.getPosition());
	sf::RenderWindow::draw(playerDot);
	
	for (auto l = player.linesArray.begin(); l != player.linesArray.end(); ++l) {
		sf::RenderWindow::draw(**l);
	}
}
void MyRenderWindow::draw(BackgroundImage& bcg) { //w ramach klasy BackgroundImage tworzony jest obiekt bcg, 
	sf::RenderWindow::draw(bcg.sprit); //on jest referencj¹!
}
void MyRenderWindow::draw(sf::Shape& s) {
	sf::RenderWindow::draw(s);
}
void MyRenderWindow::draw(sf::VertexArray& v) {
	sf::RenderWindow::draw(v);
}
