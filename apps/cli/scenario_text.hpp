#pragma once

#include "maze/grid_map.hpp"

#include <istream>

namespace maze::cli {

struct TextScenario {
    GridMap map;
    NodeId start;
    NodeId goal;
};

// Transitional input format: # wall, . floor, 1-9 terrain, S/G endpoints,
// B for start == goal. JSON project formats will have separate versioned schemas.
TextScenario read_text_scenario(std::istream& input);

} // namespace maze::cli
