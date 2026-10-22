#include "MainController.h"
#include <iostream>
#include <algorithm>
#include "DrawInterface.h"

// Constructeur
MainController::MainController(Fenetre& F, int rows, int cols)
    : rows(rows), cols(cols), gridOrigin(250, 10) // Initialise les dimensions et l'origine de la grille
{
    // Calcul de la taille dynamique des cellules
    cellSize = std::min(
        F.getScreenW() / (cols + 10), // Largeur maximale en fonction de l'écran
        F.getScreenH() / (rows + 2)  // Hauteur maximale en fonction de l'écran
    );

    // Redimensionne la grille
    grid.resize(rows, std::vector<Cell>(cols));

    fontLoaded = font.loadFromFile("Fonts/arial.ttf");
    if (!fontLoaded) {
        std::cerr << "Erreur de chargement de la police !" << std::endl;
    }

    // Initialise les différentes parties de l'application
    initializeGrid();
    initializeChecklist();
    initializeButtons();
}

// Initialisation de la grille
void MainController::initializeGrid()
{
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            // Calcule dynamiquement la position de chaque cellule
            float x = gridOrigin.x + j * cellSize;
            float y = gridOrigin.y + i * cellSize;
            grid[i][j] = Cell(i, j, x, y, cellSize, cellSize); // Initialise une cellule avec ses dimensions et sa position
        }
    }
}

// Dessiner la grille
void MainController::drawGrid(Fenetre& F)
{
	DrawInterface::drawgrid(F, grid);
}

// Gestion des clics de souris
void MainController::handleMouseClick(
    Fenetre& F,
    Cell& start,
    Cell& end,
    bool& startPointDefined,
    bool& endPointDefined,
    sf::Vector2f mousePos)
{

    // Gestion des clics sur les boutons
    for (auto& button : Buttons) {
        if (button->isClicked(mousePos)) { // Vérifie si un bouton a été cliqué
            button->onClick(Buttons,grid, checklist, start, end,F, startPointDefined, endPointDefined); // Exécute l'action associée
            return; // Arrête la vérification si un bouton a été cliqué
        }
    }

    // Gestion des clics sur les éléments de la checklist
    ChecklistItem::handleClick(mousePos, checklist);

    // Une seule case par clic, sans chevauchement des contours graphiques.
    if (mousePos.x >= gridOrigin.x && mousePos.y >= gridOrigin.y) {
        const int col = static_cast<int>((mousePos.x - gridOrigin.x) / cellSize);
        const int row = static_cast<int>((mousePos.y - gridOrigin.y) / cellSize);
        if (row < rows && col < cols) {
            grid[row][col].handleClick(startPointDefined, endPointDefined, start, end);
        }
    }
}

// Initialisation de la checklist
void MainController::initializeChecklist()
{
    if (!fontLoaded) return;

    // Création de la checklist avec des positions dynamiques
    checklist = {
        ChecklistItem(font, "BFS", sf::Vector2f(40, 50)),       // Option pour BFS
        ChecklistItem(font, "DFS", sf::Vector2f(40, 120)),      // Option pour DFS
        ChecklistItem(font, "Dijkstra", sf::Vector2f(40, 190)), // Option pour Dijkstra
        ChecklistItem(font, "A_star", sf::Vector2f(40, 260))    // Option pour A*
    };
}

// Initialisation des boutons
void MainController::initializeButtons()
{
    float buttonWidth = 150;  // Largeur du bouton
    float buttonHeight = 50;  // Hauteur du bouton

    Buttons.clear();
    ownedButtons.clear();
    if (!fontLoaded) return;
    auto addButton = [&](const char* text, float y) {
        ownedButtons.push_back(std::make_unique<Button>(font, text, sf::Vector2f(40, y),
            sf::Vector2f(buttonWidth, buttonHeight)));
        Buttons.push_back(ownedButtons.back().get());
    };
    addButton("Traitement", 350);
    addButton("Map Aleatoire", 420);
    addButton("Clean Map", 490);
}

// Dessiner la checklist
void MainController::drawChecklist(Fenetre& F)
{
	DrawInterface::drawChecklist(F, checklist);
}

// Dessiner les boutons
void MainController::drawButtons(Fenetre& F)
{
	DrawInterface::drawButtons(F, Buttons);
}
