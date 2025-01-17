#pragma once
#include <vector>
#include "Cell.h"
#include "Fenetre.h"
#include "ChecklistItem.h"
#include <SFML/Graphics.hpp>
#include "Button.h"

class MainController
{
private:
    int rows;                      // Nombre de lignes
    int cols;                      // Nombre de colonnes
    float cellSize;                // Taille dynamique des cellules
    sf::Vector2f gridOrigin;       // Origine de la grille
    std::vector<std::vector<Cell>> grid;
    std::vector<ChecklistItem> checklist;
    std::vector<Button*> Buttons;

public:

    MainController(Fenetre& F, int rows, int cols);

    void initializeGrid(Fenetre& F);
    void drawGrid(Fenetre& F);
    void handleMouseClick(
        Fenetre& F,
        Cell& start,
        Cell& end,
        bool& startPointDefined,
        bool& endPointDefined);

    void initializeChecklist();      // Initialiser la checklist
    void initializeButtons();        // Initialiser les boutons
    void drawChecklist(Fenetre& F);
    void drawButtons(Fenetre& F);
};
