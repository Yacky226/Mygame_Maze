#pragma once

#include "Cell.h"
#include "maze/search.hpp"

#include <utility>
#include <vector>

// Compatibility adapter: all searches are implemented in maze_core.
class Pathfinding {
public:
    using Result = std::pair<std::vector<Cell*>, std::vector<Cell*>>;
    static Result run(std::vector<std::vector<Cell>>& grid, Cell& start, Cell& end, maze::Algorithm algorithm);
    static Result bfs(std::vector<std::vector<Cell>>& grid, Cell& start, Cell& end);
    static Result dfs(std::vector<std::vector<Cell>>& grid, Cell& start, Cell& end);
    static Result dijkstra(std::vector<std::vector<Cell>>& grid, Cell& start, Cell& end);
    static Result astar(std::vector<std::vector<Cell>>& grid, Cell& start, Cell& end);
    static std::vector<Cell*> getNeighbors(std::vector<std::vector<Cell>>& grid, Cell& cell);
};
