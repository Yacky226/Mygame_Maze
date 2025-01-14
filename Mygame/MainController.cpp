#include "MainController.h"

void MainController::initializeGrid(std::vector<std::vector<Cell>>& grid, int rows, int cols, float cellSize, sf::Vector2f gridOrigin) {
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            // Calcule dynamiquement la position de chaque cellule
            float x = gridOrigin.x + j * cellSize;
            float y = gridOrigin.y + i * cellSize;
            Cell c(i,j,x, y, cellSize, cellSize);
            grid[i][j] = c;
        }
    }
}


void MainController::drawGrid(sf::RenderWindow& window, const std::vector<std::vector<Cell>>& grid) {
    for (const auto& row : grid) {
        for (const auto& cell : row) {
            window.draw(cell.cel); // Dessine chaque cellule
        }
    }
}


void MainController::handleMouseClick(sf::RenderWindow& window, Fenetre& F, std::vector<std::vector<Cell>>& grid,
    std::vector<ChecklistItem>& checklist, std::vector<Button*>& Buttons,
    Cell&start, Cell&end, bool& startPointDefined, bool& endPointDefined) {
    
    sf::Vector2f worldPos = F.getPositionMouse(window);

    // Gestion des boutons
    for (auto& button : Buttons) {
        if (button->isClicked(worldPos)) {
           
            button->onClick(grid, checklist, start, end,window,startPointDefined,endPointDefined);
            return; // Arrête ici si un bouton a été cliqué
        }
    }

    // Gestion de la checklist
    ChecklistItem::handleClick(worldPos, checklist);

    // Gestion des clics sur les cellules
    for (auto& row : grid) {
        for (auto& cell : row) {
            if (cell.cel.getGlobalBounds().contains(worldPos)) {
                cell.handleClick(startPointDefined, endPointDefined, start, end);
            }
        }
    }
}

