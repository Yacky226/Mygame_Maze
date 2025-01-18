#include "Fenetre.h"


//Constructeur
Fenetre::Fenetre(int ScreenW,int ScreenH):ScreenH(ScreenH), ScreenW(ScreenW),window(sf::VideoMode(ScreenW, ScreenH), "Maze")
{ }

//Retournzer lal fen�tre
sf::RenderWindow& Fenetre::getWindow()
{
	return window;
}

//retoune la largeur de la fen�tre
int Fenetre::getScreenW()
{
	return ScreenW;
}

//Retourne la Hauteur de la fen�tre
int Fenetre::getScreenH()
{
	return ScreenH;
}
//Retourne la position d'un element sur la fen�tre
sf::Vector2f Fenetre::getPositionMouse(sf::RenderWindow& window)
{
	sf::Vector2i pixelPos = sf::Mouse::getPosition(window);//Recuperer la position de la souri dans la fen�tre
	sf::Vector2f worldPos = window.mapPixelToCoords(pixelPos);//Convertir cette position en Coordonn�e
	return worldPos;// Retourner la position
}


