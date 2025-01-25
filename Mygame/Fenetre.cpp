#include "Fenetre.h"

// Constructeur
Fenetre::Fenetre(int ScreenW, int ScreenH)
    : ScreenH(ScreenH), ScreenW(ScreenW), window(sf::VideoMode(ScreenW, ScreenH), "Maze")
{
    // Initialisation de la fen�tre avec une taille donn�e (ScreenW x ScreenH) et un titre ("Maze").
    // `ScreenH` et `ScreenW` sont des attributs pour stocker les dimensions de la fen�tre.
}

// Retourne une r�f�rence � l'objet RenderWindow
sf::RenderWindow& Fenetre::getWindow()
{
    // Permet d'acc�der directement � l'objet de type `sf::RenderWindow` pour manipuler la fen�tre.
    return window;
}

// Retourne la largeur de la fen�tre
int Fenetre::getScreenW()
{
    // Fournit la largeur actuelle de la fen�tre.
    return ScreenW;
}

// Retourne la hauteur de la fen�tre
int Fenetre::getScreenH()
{
    // Fournit la hauteur actuelle de la fen�tre.
    return ScreenH;
}

// Retourne la position de la souris dans la fen�tre
sf::Vector2f Fenetre::getPositionMouse(sf::RenderWindow& window)
{
    // R�cup�re la position actuelle de la souris en pixels par rapport � la fen�tre.
    sf::Vector2i pixelPos = sf::Mouse::getPosition(window);

    // Convertit cette position en coordonn�es 
    sf::Vector2f worldPos = window.mapPixelToCoords(pixelPos);

    // Retourne la position convertie en coordonn�es
    return worldPos;
}
