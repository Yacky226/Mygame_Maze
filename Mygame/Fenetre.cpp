#include "Fenetre.h"



Fenetre::Fenetre(int ScreenW,
int ScreenH,
sf::Color colorfe):ScreenH(ScreenH), ScreenW(ScreenW),colorfe(colorfe),window(sf::VideoMode(ScreenW, ScreenH), "Maze")
{ }

sf::RenderWindow& Fenetre::getWindow()
{
	return window;
}

int Fenetre::getScreenW()
{
	return ScreenW;
}
int Fenetre::getScreenH()
{
	return ScreenH;
}

void Fenetre::draw()
{
}

sf::Vector2f Fenetre::getPositionMouse(sf::RenderWindow& window)
{
	sf::Vector2i pixelPos = sf::Mouse::getPosition(window);
	sf::Vector2f worldPos = window.mapPixelToCoords(pixelPos);
	return worldPos;
}


