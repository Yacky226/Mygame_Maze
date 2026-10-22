#include "maze/grid_map.hpp"

#include <algorithm>
#include <stdexcept>

namespace maze {
namespace {
std::size_t checked_size(std::uint32_t width, std::uint32_t height) {
    const auto count = static_cast<std::uint64_t>(width) * height;
    if (width == 0 || height == 0 || count > GridMap::max_cells) {
        throw std::invalid_argument("Map dimensions must contain 1 to 1048576 cells");
    }
    return static_cast<std::size_t>(count);
}
} // namespace

GridMap::GridMap(std::uint32_t width, std::uint32_t height)
    : width_(width), height_(height), blocked_(checked_size(width, height), 0),
      terrain_weights_(blocked_.size(), 1) {}

void GridMap::require_node(NodeId node) const {
    if (!contains(node)) {
        throw std::out_of_range("Node outside map");
    }
}

NodeId GridMap::node_at(std::uint32_t x, std::uint32_t y) const {
    if (x >= width_ || y >= height_) {
        throw std::out_of_range("Coordinates outside map");
    }
    return y * width_ + x;
}

Position GridMap::position(NodeId node) const {
    require_node(node);
    return {node % width_, node / width_};
}

bool GridMap::blocked(NodeId node) const {
    require_node(node);
    return blocked_[node] != 0;
}

void GridMap::set_blocked(NodeId node, bool blocked) {
    require_node(node);
    blocked_[node] = static_cast<std::uint8_t>(blocked);
}

std::uint32_t GridMap::terrain_weight(NodeId node) const {
    require_node(node);
    return terrain_weights_[node];
}

void GridMap::set_terrain_weight(NodeId node, std::uint32_t weight) {
    require_node(node);
    if (weight == 0 || weight > max_terrain_weight) {
        throw std::invalid_argument("Terrain weight must be between 1 and 1000");
    }
    terrain_weights_[node] = weight;
}

std::uint32_t GridMap::minimum_terrain_weight() const noexcept {
    std::uint32_t minimum = max_terrain_weight;
    for (std::size_t node = 0; node < size(); ++node) {
        if (!blocked_[node]) {
            minimum = std::min(minimum, terrain_weights_[node]);
        }
    }
    return minimum;
}

bool valid_rules(const MovementRules& rules) noexcept {
    if ((rules.connectivity != Connectivity::Four && rules.connectivity != Connectivity::Eight) ||
        (rules.corners != CornerPolicy::ForbidCutting && rules.corners != CornerPolicy::AllowCutting)) {
        return false;
    }
    // Bounds keep every possible path and heuristic far below infinite_cost.
    if (rules.orthogonal_cost == 0 || rules.orthogonal_cost > 1000000) {
        return false;
    }
    return rules.connectivity == Connectivity::Four ||
        (rules.diagonal_cost >= rules.orthogonal_cost &&
         rules.diagonal_cost <= 2 * rules.orthogonal_cost);
}

bool GridMap::uniform_costs(const MovementRules& rules) const {
    if (!valid_rules(rules) || (rules.connectivity == Connectivity::Eight &&
        rules.diagonal_cost != rules.orthogonal_cost)) {
        return false;
    }
    std::uint32_t weight = 0;
    for (std::size_t node = 0; node < size(); ++node) {
        if (!blocked_[node]) {
            if (weight != 0 && weight != terrain_weights_[node]) {
                return false;
            }
            weight = terrain_weights_[node];
        }
    }
    return true;
}

Neighbors GridMap::neighbors(NodeId node, const MovementRules& rules) const {
    require_node(node);
    if (!valid_rules(rules)) {
        throw std::invalid_argument("Invalid movement rules");
    }
    Neighbors result;
    if (blocked_[node]) {
        return result;
    }
    const auto p = position(node);
    // Stable ordering is part of deterministic tie-breaking.
    constexpr int dx[] = {-1, 1, 0, 0, -1, 1, -1, 1};
    constexpr int dy[] = {0, 0, -1, 1, -1, -1, 1, 1};
    const int count = rules.connectivity == Connectivity::Eight ? 8 : 4;
    for (int direction = 0; direction < count; ++direction) {
        const auto x = static_cast<std::int64_t>(p.x) + dx[direction];
        const auto y = static_cast<std::int64_t>(p.y) + dy[direction];
        if (x < 0 || y < 0 || x >= width_ || y >= height_) {
            continue;
        }
        const NodeId target = node_at(static_cast<std::uint32_t>(x), static_cast<std::uint32_t>(y));
        if (blocked_[target]) {
            continue;
        }
        const bool diagonal = direction >= 4;
        if (diagonal && rules.corners == CornerPolicy::ForbidCutting &&
            (blocked_[node_at(static_cast<std::uint32_t>(x), p.y)] ||
             blocked_[node_at(p.x, static_cast<std::uint32_t>(y))])) {
            continue;
        }
        const Cost base = diagonal ? rules.diagonal_cost : rules.orthogonal_cost;
        result.values[result.count++] = {target, base * terrain_weights_[target]};
    }
    return result;
}

Cost add_cost(Cost first, Cost second) noexcept {
    return first >= infinite_cost || second >= infinite_cost || first > infinite_cost - second
        ? infinite_cost : first + second;
}

PathValidation validate_path(const GridMap& map, const MovementRules& rules,
    NodeId start, NodeId goal, std::span<const NodeId> path) {
    if (!valid_rules(rules) || !map.contains(start) || !map.contains(goal) ||
        path.empty() || path.front() != start || path.back() != goal) {
        return {false, infinite_cost, "Invalid path endpoints or rules"};
    }
    Cost cost = 0;
    for (std::size_t index = 0; index < path.size(); ++index) {
        if (!map.contains(path[index]) || map.blocked(path[index])) {
            return {false, infinite_cost, "Path crosses an invalid or blocked node"};
        }
        if (index == 0) {
            continue;
        }
        bool found = false;
        for (const auto neighbor : map.neighbors(path[index - 1], rules)) {
            if (neighbor.node == path[index]) {
                cost = add_cost(cost, neighbor.cost);
                found = true;
                break;
            }
        }
        if (!found || cost == infinite_cost) {
            return {false, infinite_cost, "Path contains an invalid transition or cost overflow"};
        }
    }
    return {true, cost, {}};
}

} // namespace maze
