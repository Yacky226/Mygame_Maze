#pragma once

#include <vector>
#include "Cell.h"
#include "Fenetre.h"
#include "ChecklistItem.h"
#include <SFML/Graphics.hpp>
#include "Button.h"

// Classe principale pour gérer le jeu
class MainController
{
private:
    int rows;                      // Nombre de lignes dans la grille
    int cols;                      // Nombre de colonnes dans la grille
    float cellSize;                // Taille dynamique des cellules
    sf::Vector2f gridOrigin;       // Position d'origine de la grille sur l'écran
    std::vector<std::vector<Cell>> grid; // Grille composée d'objets `Cell`
    std::vector<ChecklistItem> checklist; // Liste d'éléments de checklist
    std::vector<Button*> Buttons;         // Liste de pointeurs vers des boutons

public:
    // Constructeur : initialise le contrôleur principal avec une fenêtre, le nombre de lignes et de colonnes
    MainController(Fenetre& F, int rows, int cols);

    // Méthode pour initialiser la grille
    void initializeGrid();

    // Méthode pour dessiner la grille dans la fenêtre
    void drawGrid(Fenetre& F);

    // Gère les clics de souris sur la grille, définit les points de départ et d'arrivée
    void handleMouseClick(
        Fenetre& F,
        Cell& start,
        Cell& end,
        bool& startPointDefined,
        bool& endPointDefined);

    // Initialiser la checklist avec des options
    void initializeChecklist();

    // Initialiser les boutons
    void initializeButtons();

    // Dessiner la checklist dans la fenêtre
    void drawChecklist(Fenetre& F);

    // Dessiner les boutons dans la fenêtre
    void drawButtons(Fenetre& F);
};
