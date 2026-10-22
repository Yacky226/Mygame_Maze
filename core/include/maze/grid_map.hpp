#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <string>
#include <vector>

namespace maze {

using NodeId = std::uint32_t;
using Cost = std::uint64_t;
inline constexpr NodeId invalid_node = std::numeric_limits<NodeId>::max();
inline constexpr Cost infinite_cost = std::numeric_limits<Cost>::max() / 4;

struct Position {
    std::uint32_t x;
    std::uint32_t y;
    bool operator==(const Position&) const = default;
};

enum class Connectivity { Four, Eight };
enum class CornerPolicy { ForbidCutting, AllowCutting };

struct MovementRules {
    Connectivity connectivity = Connectivity::Four;
    CornerPolicy corners = CornerPolicy::ForbidCutting;
    Cost orthogonal_cost = 1000;
    Cost diagonal_cost = 1414;
};

struct Neighbor {
    NodeId node;
    Cost cost;
};

struct Neighbors {
    std::array<Neighbor, 8> values{};
    std::size_t count = 0;
    auto begin() const { return values.begin(); }
    auto end() const { return values.begin() + static_cast<std::ptrdiff_t>(count); }
};

// Durable map data only. Search state and rendering never live in this class.
class GridMap {
public:
    static constexpr std::size_t max_cells = 1024 * 1024;
    static constexpr std::uint32_t max_terrain_weight = 1000;

    GridMap(std::uint32_t width, std::uint32_t height);

    std::uint32_t width() const noexcept { return width_; }
    std::uint32_t height() const noexcept { return height_; }
    std::size_t size() const noexcept { return blocked_.size(); }
    bool contains(NodeId node) const noexcept { return node < size(); }
    NodeId node_at(std::uint32_t x, std::uint32_t y) const;
    Position position(NodeId node) const;
    bool blocked(NodeId node) const;
    void set_blocked(NodeId node, bool blocked);
    std::uint32_t terrain_weight(NodeId node) const;
    void set_terrain_weight(NodeId node, std::uint32_t weight);
    std::uint32_t minimum_terrain_weight() const noexcept;
    bool uniform_costs(const MovementRules& rules) const;
    Neighbors neighbors(NodeId node, const MovementRules& rules) const;

private:
    void require_node(NodeId node) const;
    std::uint32_t width_;
    std::uint32_t height_;
    std::vector<std::uint8_t> blocked_;
    std::vector<std::uint32_t> terrain_weights_;
};

bool valid_rules(const MovementRules& rules) noexcept;
Cost add_cost(Cost first, Cost second) noexcept;

struct PathValidation {
    bool valid = false;
    Cost cost = infinite_cost;
    std::string message;
};

PathValidation validate_path(const GridMap& map, const MovementRules& rules,
    NodeId start, NodeId goal, std::span<const NodeId> path);

} // namespace maze
