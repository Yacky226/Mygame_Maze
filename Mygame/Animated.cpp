#include "Animated.h"
#include <algorithm>

namespace {
// Advance according to elapsed time, independently of rendering performance.
void animateCells(const std::vector<Cell*>& cells, sf::Color color, float secondsPerCell,
    std::vector<Button*>& buttons, std::vector<ChecklistItem>& checklist,
    Fenetre& F, std::vector<std::vector<Cell>>& grid)
{
    if (cells.empty()) {
        return;
    }
    auto& window = F.getWindow();
    sf::Clock animationClock;
    std::size_t next = 0;
    // Cap each phase at about three seconds, including long DFS paths.
    const float interval = std::min(std::max(0.001f, secondsPerCell),
        3.f / static_cast<float>(cells.size()));
    const sf::Time frameDuration = sf::seconds(1.f / 30.f);

    while (next < cells.size() && window.isOpen()) {
        sf::Clock frameClock;
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
                return;
            }
        }
        const auto due = static_cast<std::size_t>(animationClock.getElapsedTime().asSeconds() / interval);
        const auto target = std::min(cells.size(), due + 1);
        if (target > next) {
            while (next < target) {
                Cell* cell = cells[next++];
                if (cell->getType() != CellType::StartPoint && cell->getType() != CellType::EndPoint) {
                    cell->updateColor(color);
                }
            }
            window.clear(sf::Color::Black);
            DrawInterface::drawgrid(F, grid);
            DrawInterface::drawChecklist(F, checklist);
            DrawInterface::drawButtons(F, buttons);
            window.display();
        }
        // Sleep instead of busy-waiting; at most 30 frames per second.
        const sf::Time remaining = frameDuration - frameClock.getElapsedTime();
        if (remaining > sf::Time::Zero) {
            sf::sleep(remaining);
        }
    }
}
}

void Animated::animatePath(const std::vector<Cell*>& path, std::vector<Button*>& Buttons,
    std::vector<ChecklistItem>& checklist, Cell* end, Fenetre& F,
    float durationPerCell, std::vector<std::vector<Cell>>& grid)
{
    animateCells(path, sf::Color(50, 205, 50), durationPerCell, Buttons, checklist, F, grid);
}

void Animated::animateCels(const std::vector<Cell*>& visitedOrder, std::vector<Button*>& Buttons,
    std::vector<std::vector<Cell>>& grid, std::vector<ChecklistItem>& checklist,
    Cell& start, Cell& end, Fenetre& F)
{
    // 500 visited cells per second, grouped into frames (about 3 s for the full grid).
    animateCells(visitedOrder, sf::Color(200, 100, 50), 0.002f, Buttons, checklist, F, grid);
}
