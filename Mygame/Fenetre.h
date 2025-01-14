#pragma once
#include<SFML/Graphics.hpp>

struct Fenetre {
	int ScreenW;
	int ScreenH;
	sf::Color colorfe;
	Fenetre(int ScreenW,
		int ScreenH,
	sf::Color colorfe);
	void draw();
	sf::Vector2f getPositionMouse(sf::RenderWindow& window);
};
