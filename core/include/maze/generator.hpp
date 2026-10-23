#pragma once

#include "maze/grid_map.hpp"
#include <string_view>

namespace maze {
enum class Generator { Backtracker, Prim, Kruskal };
inline constexpr std::uint32_t generator_version = 1;

struct GenerationRequest {
    std::uint32_t width = 41;
    std::uint32_t height = 25;
    std::uint32_t seed = 42;
    Generator algorithm = Generator::Backtracker;
};

struct GeneratedMaze {
    GridMap map;
    NodeId start;
    NodeId goal;
};

// Odd dimensions >= 3, within GridMap::max_cells. All borders stay closed.
// Version + algorithm + dimensions + uint32 seed determine the exact output
// across native and WASM builds; no standard-library-dependent distributions.
GeneratedMaze generate_maze(const GenerationRequest& request);
std::string_view to_string(Generator algorithm) noexcept;
} // namespace maze
