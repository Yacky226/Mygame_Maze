#pragma once

#include "maze/grid_map.hpp"

#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace maze {

enum class Algorithm { Bfs, Dfs, Dijkstra, AStar };
enum class Heuristic { Automatic, Zero };
enum class SearchStatus {
    Running, Success, NoPath, Cancelled, ResourceLimit, InvalidRequest,
    UnsupportedConfiguration, InternalError
};

struct SearchRequest {
    Algorithm algorithm = Algorithm::AStar;
    NodeId start = invalid_node;
    NodeId goal = invalid_node;
    MovementRules movement;
    Heuristic heuristic = Heuristic::Automatic;
    bool record_expansions = true;
    std::uint64_t max_expansions = 0; // zero means no additional expansion limit
};

struct SearchMetrics {
    std::uint64_t discovered_unique = 0;
    std::uint64_t expanded_total = 0;
    std::uint64_t relaxations = 0;
    std::uint64_t reopened = 0;
    std::uint64_t stale_pops = 0;
    std::uint64_t frontier_peak = 0;
    std::uint64_t compute_active_ns = 0;
};

struct SearchResult {
    SearchStatus status = SearchStatus::Running;
    Cost cost = infinite_cost;
    bool optimal_cost_guaranteed = false;
    std::vector<NodeId> path;
    std::vector<NodeId> expanded_order;
    SearchMetrics metrics;
    std::string message;
};

// Owns a map snapshot and its search state. Separate sessions cannot interfere.
class SearchSession {
public:
    SearchSession(GridMap map, SearchRequest request);
    ~SearchSession();
    SearchSession(SearchSession&&) noexcept;
    SearchSession& operator=(SearchSession&&) noexcept;
    SearchSession(const SearchSession&) = delete;
    SearchSession& operator=(const SearchSession&) = delete;

    // Budget counts frontier removals, including stale heap entries.
    SearchStatus step(std::size_t max_operations = 256);
    void cancel() noexcept;
    const SearchResult& result() const noexcept;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

SearchResult search(GridMap map, SearchRequest request);
std::string_view to_string(Algorithm algorithm) noexcept;
std::string_view to_string(SearchStatus status) noexcept;

} // namespace maze
