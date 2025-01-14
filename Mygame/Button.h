#pragma once
#include<iostream>
#include<SFML/Graphics.hpp> 
#include "Cell.h"
#include "ChecklistItem.h"


// Structure pour le bouton
struct Button 
{
    sf::RectangleShape shape;
    sf::Text label;

    Button(sf::Font& font, const std::string& text,
        const sf::Vector2f& position, const sf::Vector2f& size);

    void draw(sf::RenderWindow& window);

    bool isClicked(const sf::Vector2f& mousePos);

    void onClick(std::vector<std::vector<Cell>>& grid, const std::vector<ChecklistItem>& checklist,Cell &start,Cell &end, sf::RenderWindow& window, bool& startPointDefined, bool& endPointDefined);

    void generateRandomMap(std::vector<std::vector<Cell>>& grid, Cell& start, Cell& end);
    void handleChecklistSelection(std::vector<std::vector<Cell>>& grid,const std::vector<ChecklistItem>& checklist,Cell &start, Cell &end, sf::RenderWindow& window);
    void cleanMap(std::vector<std::vector<Cell>>& grid,Cell &start, Cell &end);
    void bfs(std::vector<std::vector<Cell>>& grid, Cell &start, Cell &end, sf::RenderWindow& window);
    void dfs(std::vector<std::vector<Cell>>& grid, Cell& start, Cell& end, sf::RenderWindow& window);
    void dijkstra(std::vector<std::vector<Cell>>& grid, Cell& start, Cell& end, sf::RenderWindow& window);
    std::vector<Cell*> getNeighbors(std::vector<std::vector<Cell>>& grid, Cell& cell);
    void tracePath(Cell& end);
    void animatePath(Cell* end, sf::RenderWindow& window, float durationPerCell, std::vector<std::vector<Cell>>& grid);
    void astar(std::vector<std::vector<Cell>>& grid, Cell& start, Cell& end, sf::RenderWindow& window);
    float manhattanDistance(const Cell& a, const Cell& b);
};
