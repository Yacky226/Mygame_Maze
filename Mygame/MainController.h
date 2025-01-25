#pragma once

#include <vector>
#include "Cell.h"
#include "Fenetre.h"
#include "ChecklistItem.h"
#include <SFML/Graphics.hpp>
#include "Button.h"

// Classe principale pour g�rer le jeu
class MainController
{
private:
    int rows;                      // Nombre de lignes dans la grille
    int cols;                      // Nombre de colonnes dans la grille
    float cellSize;                // Taille dynamique des cellules
    sf::Vector2f gridOrigin;       // Position d'origine de la grille sur l'�cran
    std::vector<std::vector<Cell>> grid; // Grille compos�e d'objets `Cell`
    std::vector<ChecklistItem> checklist; // Liste d'�l�ments de checklist
    std::vector<Button*> Buttons;         // Liste de pointeurs vers des boutons

public:
    // Constructeur : initialise le contr�leur principal avec une fen�tre, le nombre de lignes et de colonnes
    MainController(Fenetre& F, int rows, int cols);

    // M�thode pour initialiser la grille
    void initializeGrid();

    // M�thode pour dessiner la grille dans la fen�tre
    void drawGrid(Fenetre& F);

    // G�re les clics de souris sur la grille, d�finit les points de d�part et d'arriv�e
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

    // Dessiner la checklist dans la fen�tre
    void drawChecklist(Fenetre& F);

    // Dessiner les boutons dans la fen�tre
    void drawButtons(Fenetre& F);
};
