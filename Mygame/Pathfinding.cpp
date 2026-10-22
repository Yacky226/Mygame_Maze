#include "Pathfinding.h"

#include <algorithm>
#include <iostream>

Pathfinding::Result Pathfinding::run(std::vector<std::vector<Cell>>& grid,
    Cell& start, Cell& end, maze::Algorithm algorithm)
{
    if (grid.empty() || grid.front().empty()) return {};
    const auto rows = grid.size();
    const auto cols = grid.front().size();
    if (cols > maze::GridMap::max_cells || rows > maze::GridMap::max_cells ||
        static_cast<std::uint64_t>(rows) * cols > maze::GridMap::max_cells ||
        std::any_of(grid.begin(), grid.end(), [cols](const auto& row) { return row.size() != cols; })) return {};
    auto valid = [&](const Cell& cell) {
        return cell.indexX >= 0 && cell.indexY >= 0 &&
            static_cast<std::size_t>(cell.indexX) < rows && static_cast<std::size_t>(cell.indexY) < cols;
    };
    if (!valid(start) || !valid(end) ||
        grid[start.indexX][start.indexY].getType() != StartPoint ||
        grid[end.indexX][end.indexY].getType() != EndPoint) return {};

    maze::GridMap map(static_cast<std::uint32_t>(cols), static_cast<std::uint32_t>(rows));
    for (std::size_t row = 0; row < rows; ++row) {
        for (std::size_t col = 0; col < cols; ++col) {
            auto& cell = grid[row][col];
            const auto type = cell.getType();
            map.set_blocked(map.node_at(static_cast<std::uint32_t>(col), static_cast<std::uint32_t>(row)), type == Wall);
            // Clear the previous visualization and legacy search fields only.
            cell.clean();
            cell.setType(type);
        }
    }
    maze::SearchRequest request;
    request.algorithm = algorithm;
    // Legacy indexX is the row; the new core consistently uses x = column.
    request.start = map.node_at(start.indexY, start.indexX);
    request.goal = map.node_at(end.indexY, end.indexX);
    const auto result = maze::search(map, request);
    Result converted;
    auto append = [&](const std::vector<maze::NodeId>& nodes, std::vector<Cell*>& cells) {
        cells.reserve(nodes.size());
        for (const auto node : nodes) {
            const auto position = map.position(node);
            cells.push_back(&grid[position.y][position.x]);
        }
    };
    append(result.expanded_order, converted.first);
    append(result.path, converted.second);
    std::cout << "[maze_core] " << maze::to_string(algorithm) << " status=" << maze::to_string(result.status)
              << " expanded=" << result.metrics.expanded_total;
    if (result.status == maze::SearchStatus::Success) std::cout << " cost=" << result.cost;
    std::cout << std::endl;
    return converted;
}

Pathfinding::Result Pathfinding::bfs(std::vector<std::vector<Cell>>& grid, Cell& start, Cell& end) {
    return run(grid, start, end, maze::Algorithm::Bfs);
}
Pathfinding::Result Pathfinding::dfs(std::vector<std::vector<Cell>>& grid, Cell& start, Cell& end) {
    return run(grid, start, end, maze::Algorithm::Dfs);
}
Pathfinding::Result Pathfinding::dijkstra(std::vector<std::vector<Cell>>& grid, Cell& start, Cell& end) {
    return run(grid, start, end, maze::Algorithm::Dijkstra);
}
Pathfinding::Result Pathfinding::astar(std::vector<std::vector<Cell>>& grid, Cell& start, Cell& end) {
    return run(grid, start, end, maze::Algorithm::AStar);
}

std::vector<Cell*> Pathfinding::getNeighbors(std::vector<std::vector<Cell>>& grid, Cell& cell) {
    std::vector<Cell*> neighbors;
    const int row = cell.indexX;
    const int col = cell.indexY;
    if (row < 0 || col < 0 || static_cast<std::size_t>(row) >= grid.size() ||
        static_cast<std::size_t>(col) >= grid[row].size()) return neighbors;
    auto add = [&](int r, int c) {
        if (r >= 0 && c >= 0 && static_cast<std::size_t>(r) < grid.size() &&
            static_cast<std::size_t>(c) < grid[r].size()) neighbors.push_back(&grid[r][c]);
    };
    add(row, col - 1); add(row, col + 1); add(row - 1, col); add(row + 1, col);
    return neighbors;
}
