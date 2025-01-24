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

    // Initialise les différentes parties de l'application
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
    bool& endPointDefined)
{
    sf::RenderWindow& window = F.getWindow();
    sf::Vector2f mousePos = F.getPositionMouse(window); // Récupère la position de la souris

    // Gestion des clics sur les boutons
    for (auto& button : Buttons) {
        if (button->isClicked(mousePos)) { // Vérifie si un bouton a été cliqué
            button->onClick(Buttons,grid, checklist, start, end,F, startPointDefined, endPointDefined); // Exécute l'action associée
            return; // Arrête la vérification si un bouton a été cliqué
        }
    }

    // Gestion des clics sur les éléments de la checklist
    ChecklistItem::handleClick(mousePos, checklist);

    // Gestion des clics sur les cellules
    for (auto& row : grid) {
        for (auto& cell : row) {
            if (cell.cel.getGlobalBounds().contains(mousePos)) { // Vérifie si la souris est sur une cellule
                cell.handleClick(startPointDefined, endPointDefined, start, end); // Gère le clic sur la cellule
            }
        }
    }
}

// Initialisation de la checklist
void MainController::initializeChecklist()
{
    // Chargement de la police
    sf::Font* font = new sf::Font();
    if (!font->loadFromFile("C:/Windows/Fonts/arial.ttf")) { // Chemin vers la police
        std::cerr << "Erreur de chargement de la police pour les boutons !" << std::endl;
        return;
    }

    // Création de la checklist avec des positions dynamiques
    checklist = {
        ChecklistItem(*font, "BFS", sf::Vector2f(40, 50)),       // Option pour BFS
        ChecklistItem(*font, "DFS", sf::Vector2f(40, 120)),      // Option pour DFS
        ChecklistItem(*font, "Dijkstra", sf::Vector2f(40, 190)), // Option pour Dijkstra
        ChecklistItem(*font, "A_star", sf::Vector2f(40, 260))    // Option pour A*
    };
}

// Initialisation des boutons
void MainController::initializeButtons()
{
    float buttonWidth = 150;  // Largeur du bouton
    float buttonHeight = 50;  // Hauteur du bouton

    // Chargement de la police
    sf::Font* font = new sf::Font();
    if (!font->loadFromFile("C:/Windows/Fonts/arial.ttf")) { // Chemin vers la police
        std::cerr << "Erreur de chargement de la police pour les boutons !" << std::endl;
        return;
    }

    // Création des boutons avec leurs positions et tailles
    Buttons = {
        new Button(*font, "Traitement", sf::Vector2f(40, 350), sf::Vector2f(buttonWidth, buttonHeight)), // Bouton pour traiter la grille
        new Button(*font, "Map Aleatoire", sf::Vector2f(40, 420), sf::Vector2f(buttonWidth, buttonHeight)), // Bouton pour générer une carte aléatoire
        new Button(*font, "Clean Map", sf::Vector2f(40, 490), sf::Vector2f(buttonWidth, buttonHeight)) // Bouton pour nettoyer la carte
    };
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
