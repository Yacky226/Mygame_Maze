#include <SFML/Graphics.hpp>
#include "Fenetre.h"
#include "Cell.h"
#include "ChecklistItem.h"
#include "Button.h"
#include <vector>
#include <iostream>
#include "MainController.h"

// Fonctions auxiliaires


int main() {
    // Dimensions dynamiques de la fenêtre
    const int windowWidth = 1300;
    const int windowHeight = 800;
    Fenetre F(windowWidth, windowHeight, sf::Color::Black);
    sf::RenderWindow window(sf::VideoMode(F.ScreenW, F.ScreenH), "Maze");

    // Taille de la grille et dimensions dynamiques
    int rows = 30; // Nombre de lignes
    int cols = 50; // Nombre de colonnes
    float cellSize = std::min(windowWidth / (cols + 10), windowHeight / (rows + 2)); // Taille dynamique des cellules
    sf::Vector2f gridOrigin(250, 10); // Origine de la grille (modifiable dynamiquement)

    // Variables de la grille et des points de départ/fin
    std::vector<std::vector<Cell>> grid(rows, std::vector<Cell>(cols));
    Cell start  ;
    Cell end ;
    bool startPointDefined = false;
    bool endPointDefined = false;

    // Initialisation de la grille avec des tailles dynamiques
      MainController::initializeGrid(grid, rows, cols, cellSize, gridOrigin);

    // Chargement de la police
    sf::Font font;
    if (!font.loadFromFile("C:/Windows/Fonts/arial.ttf")) {
        std::cerr << "Erreur de chargement de la police !" << std::endl;
        return -1;
    }

    // Création de la checklist avec des positions dynamiques
    std::vector<ChecklistItem> checklist = {
        ChecklistItem(font, "BFS", sf::Vector2f(40, 50)),
        ChecklistItem(font, "DFS", sf::Vector2f(40, 120)),
        ChecklistItem(font, "Dijkstra", sf::Vector2f(40, 190)),
        ChecklistItem(font, "A_star", sf::Vector2f(40, 260))
    };

    // Création des boutons dynamiques
    float buttonWidth = 150;
    float buttonHeight = 50;
    std::vector<Button*> Buttons = {
        new Button(font, "Traitement", sf::Vector2f(40, 350), sf::Vector2f(buttonWidth, buttonHeight)),
        new Button(font, "Map Aleatoire", sf::Vector2f(40, 420), sf::Vector2f(buttonWidth, buttonHeight)),
        new Button(font, "Clean Map", sf::Vector2f(40, 490), sf::Vector2f(buttonWidth, buttonHeight))
    };

    // Boucle principale
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();

            // Gestion des clics souris
            if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
                MainController::handleMouseClick(window, F, grid, checklist, Buttons, start,end, startPointDefined, endPointDefined);
            }
        }

        // Dessin
        window.clear(sf::Color::Black);
        MainController::drawGrid(window, grid);
        for (auto& item : checklist) item.draw(window);
        for (auto& button : Buttons) button->draw(window);
        window.display();
    }

    return 0;
}
