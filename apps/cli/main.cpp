#include "maze/search.hpp"
#include "scenario_text.hpp"

#include <charconv>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

namespace {
std::string quoted(std::string_view text) {
    std::string result = "\"";
    constexpr char hex[] = "0123456789abcdef";
    for (const unsigned char c : text) {
        if (c == '"' || c == '\\') { result += '\\'; result += static_cast<char>(c); }
        else if (c < 0x20) { result += "\\u00"; result += hex[c >> 4]; result += hex[c & 15]; }
        else result += static_cast<char>(c);
    }
    return result + '"';
}

maze::Algorithm parse_algorithm(std::string_view name) {
    if (name == "bfs") return maze::Algorithm::Bfs;
    if (name == "dfs") return maze::Algorithm::Dfs;
    if (name == "dijkstra") return maze::Algorithm::Dijkstra;
    if (name == "astar") return maze::Algorithm::AStar;
    throw std::invalid_argument("Algorithm must be bfs, dfs, dijkstra or astar");
}

std::uint64_t parse_limit(std::string_view text) {
    std::uint64_t result = 0;
    const auto parsed = std::from_chars(text.data(), text.data() + text.size(), result);
    if (parsed.ec != std::errc{} || parsed.ptr != text.data() + text.size()) {
        throw std::invalid_argument("Expansion limit must be an unsigned integer");
    }
    return result;
}
} // namespace

int main(int argc, char** argv) {
    if (argc == 2 && std::string_view(argv[1]) == "--help") {
        std::cout << "Usage: maze_cli MAP [--algorithm bfs|dfs|dijkstra|astar] [--diagonal]\n"
                     "                    [--allow-corner-cutting] [--max-expansions N]\n"
                     "Map: rectangular ASCII, # wall, . floor, 1-9 cost multiplier, S/G endpoints, B both.\n"
                     "Costs: 1000 orthogonal, 1414 diagonal, multiplied by destination terrain.\n"
                     "Exit: 0 success; 2 input/configuration error; 3 no path; 4 resource limit; 5 internal error.\n";
        return 0;
    }
    try {
        if (argc < 2) throw std::invalid_argument("Missing map path; use --help");
        maze::SearchRequest request;
        request.record_expansions = false;
        for (int argument = 2; argument < argc; ++argument) {
            const std::string_view option = argv[argument];
            if (option == "--algorithm" || option == "--max-expansions") {
                if (++argument >= argc) throw std::invalid_argument("Missing option value");
                if (option == "--algorithm") request.algorithm = parse_algorithm(argv[argument]);
                else request.max_expansions = parse_limit(argv[argument]);
            } else if (option == "--diagonal") request.movement.connectivity = maze::Connectivity::Eight;
            else if (option == "--allow-corner-cutting") request.movement.corners = maze::CornerPolicy::AllowCutting;
            else throw std::invalid_argument("Unknown option");
        }
        if (request.movement.corners == maze::CornerPolicy::AllowCutting &&
            request.movement.connectivity != maze::Connectivity::Eight) {
            throw std::invalid_argument("Corner cutting requires --diagonal");
        }
        std::ifstream input(argv[1], std::ios::binary);
        if (!input) throw std::invalid_argument("Unable to open map file");
        auto scenario = maze::cli::read_text_scenario(input);
        request.start = scenario.start;
        request.goal = scenario.goal;
        const auto result = maze::search(scenario.map, request);
        if (result.status == maze::SearchStatus::Success) {
            const auto checked = maze::validate_path(scenario.map, request.movement,
                request.start, request.goal, result.path);
            if (!checked.valid || checked.cost != result.cost) {
                throw std::runtime_error("Engine returned an invalid path or cost");
            }
        }
        std::cout << "{\"schemaVersion\":1,\"engineVersion\":" << quoted(MAZE_ENGINE_VERSION)
                  << ",\"algorithm\":" << quoted(maze::to_string(request.algorithm)) << ",\"status\":"
                  << quoted(maze::to_string(result.status)) << ",\"cost\":";
        if (result.status == maze::SearchStatus::Success) std::cout << '"' << result.cost << '"';
        else std::cout << "null";
        std::cout << ",\"costUnit\":\"milli-step\",\"width\":" << scenario.map.width()
                  << ",\"height\":" << scenario.map.height()
                  << ",\"movement\":{\"connectivity\":"
                  << (request.movement.connectivity == maze::Connectivity::Four ? 4 : 8)
                  << ",\"cornerPolicy\":" << quoted(request.movement.corners == maze::CornerPolicy::ForbidCutting
                      ? "forbid_cutting" : "allow_cutting")
                  << ",\"orthogonalCost\":\"" << request.movement.orthogonal_cost
                  << "\",\"diagonalCost\":\"" << request.movement.diagonal_cost
                  << "\"},\"optimalCostGuaranteed\":"
                  << (result.optimal_cost_guaranteed ? "true" : "false")
                  << ",\"pathEdges\":" << (result.path.empty() ? 0 : result.path.size() - 1)
                  << ",\"path\":[";
        for (std::size_t index = 0; index < result.path.size(); ++index) {
            const auto position = scenario.map.position(result.path[index]);
            if (index != 0) std::cout << ',';
            std::cout << '[' << position.x << ',' << position.y << ']';
        }
        std::cout << "],\"metrics\":{\"discoveredUnique\":" << result.metrics.discovered_unique
                  << ",\"expandedTotal\":" << result.metrics.expanded_total
                  << ",\"relaxations\":" << result.metrics.relaxations
                  << ",\"frontierPeak\":" << result.metrics.frontier_peak
                  << ",\"stalePops\":" << result.metrics.stale_pops
                  << ",\"computeActiveNs\":\"" << result.metrics.compute_active_ns
                  << "\"},\"message\":" << quoted(result.message) << "}\n";
        switch (result.status) {
        case maze::SearchStatus::Success: return 0;
        case maze::SearchStatus::NoPath: return 3;
        case maze::SearchStatus::ResourceLimit: return 4;
        case maze::SearchStatus::InternalError: return 5;
        default: return 2;
        }
    } catch (const std::invalid_argument& error) {
        std::cout << "{\"status\":\"invalid_request\",\"message\":" << quoted(error.what()) << "}\n";
        return 2;
    } catch (const std::exception& error) {
        std::cout << "{\"status\":\"internal_error\",\"message\":" << quoted(error.what()) << "}\n";
        return 5;
    }
}
