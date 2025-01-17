#include "MainController.h"
#include <iostream>
#include <algorithm>

// Constructeur
MainController::MainController(Fenetre& F, int rows, int cols)
    : rows(rows), cols(cols), gridOrigin(250, 10)
{
    // Calcul de la taille dynamique des cellules
    cellSize = std::min(
        F.getScreenW() / (cols + 10),
        F.getScreenH() / (rows + 2)
    );

    // Redimensionne la grille
    grid.resize(rows, std::vector<Cell>(cols));

    initializeGrid(F);
    initializeChecklist();
    initializeButtons();
}

// Initialisation de la grille
void MainController::initializeGrid(Fenetre& F)
{
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            // Calcule dynamiquement la position de chaque cellule
            float x = gridOrigin.x + j * cellSize;
            float y = gridOrigin.y + i * cellSize;
            grid[i][j] = Cell(i, j, x, y, cellSize, cellSize);
        }
    }
}

// Dessiner la grille
void MainController::drawGrid(Fenetre& F)
{
    sf::RenderWindow& window = F.getWindow();
    for (const auto& row : grid) {
        for (const auto& cell : row) {
            window.draw(cell.cel); // Dessine chaque cellule
        }
    }
}

// Gestion des clics de souris
void MainController::handleMouseClick(
    Fenetre& F,
    Cell& start,
    Cell& end,
    bool& startPointDefined,
    bool& endPointDefined)
{
    sf::RenderWindow& window = F.getWindow();
    sf::Vector2f mousePos = F.getPositionMouse(window);

    // Gestion des boutons
    for (auto& button : Buttons) {
        if (button->isClicked(mousePos)) {
            button->onClick(grid, checklist, start, end, window, startPointDefined, endPointDefined);
            return; // Arrête ici si un bouton a été cliqué
        }
    }

    // Gestion de la checklist
    ChecklistItem::handleClick(mousePos, checklist);

    // Gestion des clics sur les cellules
    for (auto& row : grid) {
        for (auto& cell : row) {
            if (cell.cel.getGlobalBounds().contains(mousePos)) {
                cell.handleClick(startPointDefined, endPointDefined, start, end);
            }
        }
    }
}

// Initialisation de la checklist
void MainController::initializeChecklist()
{
    // Chargement de la police
    sf::Font* font = new sf::Font();
    if (!font->loadFromFile("C:/Windows/Fonts/arial.ttf")) {
        std::cerr << "Erreur de chargement de la police pour les boutons !" << std::endl;
        return;
    }

    // Création de la checklist avec des positions dynamiques
    checklist = {
        ChecklistItem(*font, "BFS", sf::Vector2f(40, 50)),
        ChecklistItem(*font, "DFS", sf::Vector2f(40, 120)),
        ChecklistItem(*font, "Dijkstra", sf::Vector2f(40, 190)),
        ChecklistItem(*font, "A_star", sf::Vector2f(40, 260))
    };
}

// Initialisation des boutons
void MainController::initializeButtons()
{
    float buttonWidth = 150;
    float buttonHeight = 50;
    // Chargement de la police
    sf::Font* font = new sf::Font();
    if (!font->loadFromFile("C:/Windows/Fonts/arial.ttf")) {
        std::cerr << "Erreur de chargement de la police pour les boutons !" << std::endl;
        return;
    }

    Buttons = {
        new Button(*font, "Traitement", sf::Vector2f(40, 350), sf::Vector2f(buttonWidth, buttonHeight)),
        new Button(*font, "Map Aleatoire", sf::Vector2f(40, 420), sf::Vector2f(buttonWidth, buttonHeight)),
        new Button(*font, "Clean Map", sf::Vector2f(40, 490), sf::Vector2f(buttonWidth, buttonHeight))
    };
}

// Dessiner la checklist
void MainController::drawChecklist(Fenetre& F)
{
    sf::RenderWindow& window = F.getWindow();
    for (auto& item : checklist) {
        item.draw(window);
    }
}

// Dessiner les boutons
void MainController::drawButtons(Fenetre& F)
{
    sf::RenderWindow& window = F.getWindow();
    for (auto& button : Buttons) {
        button->draw(window);
    }
}
