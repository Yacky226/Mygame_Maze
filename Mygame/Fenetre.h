#pragma once
#include<SFML/Graphics.hpp>

class Fenetre {
private:
	int ScreenW;
	int ScreenH;
	sf::RenderWindow window;
	sf::Color colorfe;
public:
	sf::RenderWindow& getWindow();
	int getScreenW();
	int getScreenH();
	Fenetre(int ScreenW,int ScreenH,sf::Color colorfe);
	void draw();
	sf::Vector2f getPositionMouse(sf::RenderWindow& window);
};
