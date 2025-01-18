#pragma once
#include<SFML/Graphics.hpp>

class Fenetre {
private:
	int ScreenW;                  //Largeur de la Fenêtre
	int ScreenH;		         //Hateur de la Fenêtre
	sf::RenderWindow window;      //la Fenêtre
public:
	Fenetre(int ScreenW, int ScreenH); //Construceur
	sf::RenderWindow& getWindow(); // Fonction pour retourner ma Fenêtre
	int getScreenW();   //Fonction Pour retourner la largeur de la fenêtre
	int getScreenH();   //Fonction pour retourner la hauteur de la fenètre
	sf::Vector2f getPositionMouse(sf::RenderWindow& window); //Fonction qui retourne la position d'un element de la fenêtre
};
