#include "Fenetre.h"

// Constructeur
Fenetre::Fenetre(int ScreenW, int ScreenH)
    : ScreenH(ScreenH), ScreenW(ScreenW), window(sf::VideoMode(ScreenW, ScreenH), "Maze")
{
    // Initialisation de la fenêtre avec une taille donnée (ScreenW x ScreenH) et un titre ("Maze").
    // `ScreenH` et `ScreenW` sont des attributs pour stocker les dimensions de la fenêtre.
}

// Retourne une référence à l'objet RenderWindow
sf::RenderWindow& Fenetre::getWindow()
{
    // Permet d'accéder directement à l'objet de type `sf::RenderWindow` pour manipuler la fenêtre.
    return window;
}

// Retourne la largeur de la fenêtre
int Fenetre::getScreenW()
{
    // Fournit la largeur actuelle de la fenêtre.
    return ScreenW;
}

// Retourne la hauteur de la fenêtre
int Fenetre::getScreenH()
{
    // Fournit la hauteur actuelle de la fenêtre.
    return ScreenH;
}

// Retourne la position de la souris dans la fenêtre
sf::Vector2f Fenetre::getPositionMouse(sf::RenderWindow& window)
{
    // Récupère la position actuelle de la souris en pixels par rapport à la fenêtre.
    sf::Vector2i pixelPos = sf::Mouse::getPosition(window);

    // Convertit cette position en coordonnées 
    sf::Vector2f worldPos = window.mapPixelToCoords(pixelPos);

    // Retourne la position convertie en coordonnées
    return worldPos;
}
