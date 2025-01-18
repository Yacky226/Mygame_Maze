#pragma once
#include<SFML/Graphics.hpp>

class Fenetre {
private:
	int ScreenW;                  //Largeur de la Fen�tre
	int ScreenH;		         //Hateur de la Fen�tre
	sf::RenderWindow window;      //la Fen�tre
public:
	Fenetre(int ScreenW, int ScreenH); //Construceur
	sf::RenderWindow& getWindow(); // Fonction pour retourner ma Fen�tre
	int getScreenW();   //Fonction Pour retourner la largeur de la fen�tre
	int getScreenH();   //Fonction pour retourner la hauteur de la fen�tre
	sf::Vector2f getPositionMouse(sf::RenderWindow& window); //Fonction qui retourne la position d'un element de la fen�tre
};
