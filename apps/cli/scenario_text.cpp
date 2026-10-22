#include "scenario_text.hpp"

#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace maze::cli {

TextScenario read_text_scenario(std::istream& input) {
    constexpr std::size_t max_input_bytes = 4 * 1024 * 1024;
    std::vector<std::string> rows;
    std::size_t bytes = 0;
    std::string line;
    // Read with a byte budget, including malformed lines without a newline.
    char character;
    while (input.get(character)) {
        if (++bytes > max_input_bytes) throw std::invalid_argument("Map file exceeds 4 MiB");
        if (character == '\n') {
            if (!line.empty() && line.back() == '\r') line.pop_back();
            if (line.empty() || (!rows.empty() && line.size() != rows.front().size())) {
                throw std::invalid_argument("Map rows must be nonempty and have equal widths");
            }
            if (static_cast<std::uint64_t>(rows.size() + 1) * line.size() > GridMap::max_cells) {
                throw std::invalid_argument("Map exceeds the cell limit");
            }
            rows.push_back(std::move(line));
            line.clear();
        } else {
            line.push_back(character);
            if (line.size() > GridMap::max_cells + 1) {
                throw std::invalid_argument("Map line exceeds the cell limit");
            }
        }
    }
    if (input.bad()) throw std::runtime_error("Unable to read map file");
    if (!line.empty()) {
        if (line.back() == '\r') line.pop_back();
        rows.push_back(std::move(line));
    }
    if (rows.empty() || rows.front().empty()) throw std::invalid_argument("Map must not be empty");
    const auto width = rows.front().size();
    if (width > GridMap::max_cells || rows.size() > GridMap::max_cells ||
        static_cast<std::uint64_t>(width) * rows.size() > GridMap::max_cells) {
        throw std::invalid_argument("Map exceeds the cell limit");
    }
    GridMap map(static_cast<std::uint32_t>(width), static_cast<std::uint32_t>(rows.size()));
    NodeId start = invalid_node;
    NodeId goal = invalid_node;
    for (std::size_t y = 0; y < rows.size(); ++y) {
        if (rows[y].size() != width) throw std::invalid_argument("Map rows must have equal widths");
        for (std::size_t x = 0; x < width; ++x) {
            const char symbol = rows[y][x];
            const auto node = map.node_at(static_cast<std::uint32_t>(x), static_cast<std::uint32_t>(y));
            if (symbol == 'S' || symbol == 'B') {
                if (start != invalid_node) throw std::invalid_argument("Map contains multiple starts");
                start = node;
            }
            if (symbol == 'G' || symbol == 'B') {
                if (goal != invalid_node) throw std::invalid_argument("Map contains multiple goals");
                goal = node;
            }
            if (symbol == '#') map.set_blocked(node, true);
            else if (symbol >= '1' && symbol <= '9') map.set_terrain_weight(node, symbol - '0');
            else if (symbol != '.' && symbol != 'S' && symbol != 'G' && symbol != 'B') {
                throw std::invalid_argument("Unknown map symbol; expected # . S G B or 1-9");
            }
        }
    }
    if (start == invalid_node || goal == invalid_node) {
        throw std::invalid_argument("Map requires one start and one goal (or B)");
    }
    return {std::move(map), start, goal};
}

} // namespace maze::cli
