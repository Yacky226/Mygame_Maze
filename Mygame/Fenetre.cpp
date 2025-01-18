#include "Fenetre.h"


//Constructeur
Fenetre::Fenetre(int ScreenW,int ScreenH):ScreenH(ScreenH), ScreenW(ScreenW),window(sf::VideoMode(ScreenW, ScreenH), "Maze")
{ }

//Retournzer lal fenêtre
sf::RenderWindow& Fenetre::getWindow()
{
	return window;
}

//retoune la largeur de la fenêtre
int Fenetre::getScreenW()
{
	return ScreenW;
}

//Retourne la Hauteur de la fenêtre
int Fenetre::getScreenH()
{
	return ScreenH;
}
//Retourne la position d'un element sur la fenêtre
sf::Vector2f Fenetre::getPositionMouse(sf::RenderWindow& window)
{
	sf::Vector2i pixelPos = sf::Mouse::getPosition(window);//Recuperer la position de la souri dans la fenêtre
	sf::Vector2f worldPos = window.mapPixelToCoords(pixelPos);//Convertir cette position en Coordonnée
	return worldPos;// Retourner la position
}


