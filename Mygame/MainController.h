#pragma once
#include <vector>
#include "Cell.h"
#include "Fenetre.h"
#include "ChecklistItem.h"
#include<SFML/Graphics.hpp>
#include "Button.h"

struct Point {
    int x;
    int y;
};

struct MainController
{
   static void initializeGrid(std::vector<std::vector<Cell>>& grid, int rows, int cols, float cellSize, sf::Vector2f gridOrigin);
   static void drawGrid(sf::RenderWindow& window, const std::vector<std::vector<Cell>>& grid);
   static void handleMouseClick(sf::RenderWindow& window, Fenetre& F, std::vector<std::vector<Cell>>& grid,
        std::vector<ChecklistItem>& checklist, std::vector<Button*>& Buttons,
        Cell& start, Cell &end, bool& startPointDefined, bool& endPointDefined);
};

