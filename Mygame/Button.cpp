#include "Button.h"
#include "Pathfinding.h"
#include "Animated.h"

Button::Button(sf::Font& font, const std::string& text, const sf::Vector2f& position, const sf::Vector2f& size)
{
    shape.setSize(size);
    shape.setFillColor(sf::Color::Blue);
    shape.setPosition(position);
    shape.setOutlineThickness(2);
    shape.setOutlineColor(sf::Color::Black);
    label.setFont(font);
    label.setString(text);
    label.setCharacterSize(20);
    label.setFillColor(sf::Color::White);
    const auto bounds = label.getLocalBounds();
    label.setPosition(position.x + (size.x - bounds.width) / 2 - bounds.left,
        position.y + (size.y - bounds.height) / 2 - bounds.top);
}

void Button::draw(sf::RenderWindow& window) {
    window.draw(shape);
    window.draw(label);
}

bool Button::isClicked(const sf::Vector2f& mousePos) {
    return shape.getGlobalBounds().contains(mousePos);
}

void Button::onClick(std::vector<Button*>& Buttons, std::vector<std::vector<Cell>>& grid,
    std::vector<ChecklistItem>& checklist, Cell& start, Cell& end, Fenetre& F,
    bool& startPointDefined, bool& endPointDefined)
{
    const auto action = label.getString();
    if (action == "Map Aleatoire" || action == "Clean Map") {
        startPointDefined = false;
        endPointDefined = false;
        start = Cell();
        end = Cell();
        if (action == "Map Aleatoire") DrawInterface::generateRandomMap(grid);
        else DrawInterface::cleanMap(grid);
    } else if (action == "Traitement" && startPointDefined && endPointDefined) {
        handleChecklistSelection(Buttons, grid, checklist, start, end, F);
        // Endpoints and walls remain available for another search.
    }
}

void Button::handleChecklistSelection(std::vector<Button*>& Buttons, std::vector<std::vector<Cell>>& grid,
    std::vector<ChecklistItem>& checklist, Cell& start, Cell& end, Fenetre& F)
{
    for (const auto& item : checklist) {
        if (!item.isChecked()) continue;
        maze::Algorithm algorithm;
        const auto name = item.getText();
        if (name == "BFS") algorithm = maze::Algorithm::Bfs;
        else if (name == "DFS") algorithm = maze::Algorithm::Dfs;
        else if (name == "Dijkstra") algorithm = maze::Algorithm::Dijkstra;
        else if (name == "A_star") algorithm = maze::Algorithm::AStar;
        else continue;
        const auto result = Pathfinding::run(grid, start, end, algorithm);
        Animated::animateCels(result.first, Buttons, grid, checklist, start, end, F);
        Animated::animatePath(result.second, Buttons, checklist, &end, F, 0.02f, grid);
        return;
    }
}
