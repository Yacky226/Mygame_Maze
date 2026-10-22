#include "maze/search.hpp"

#include <catch2/catch_test_macros.hpp>
#include <algorithm>
#include <array>
#include <cstdlib>
#include <limits>

using namespace maze;

namespace {
constexpr std::array algorithms{Algorithm::Bfs, Algorithm::Dfs, Algorithm::Dijkstra, Algorithm::AStar};

SearchRequest request_for(const GridMap& map, Algorithm algorithm) {
    SearchRequest request;
    request.algorithm = algorithm;
    request.start = 0;
    request.goal = static_cast<NodeId>(map.size() - 1);
    return request;
}

// Deliberately independent transition enumeration and Bellman-Ford relaxation.
// It does not call the production neighbor generator, heuristic or frontier.
Cost oracle(const GridMap& map, const SearchRequest& request) {
    std::vector<Cost> distances(map.size(), infinite_cost);
    distances[request.start] = 0;
    const int width = static_cast<int>(map.width());
    const int height = static_cast<int>(map.height());
    for (std::size_t pass = 1; pass < map.size(); ++pass) {
        bool changed = false;
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                const auto node = map.node_at(x, y);
                if (map.blocked(node) || distances[node] == infinite_cost) continue;
                for (int dy = -1; dy <= 1; ++dy) {
                    for (int dx = -1; dx <= 1; ++dx) {
                        if (dx == 0 && dy == 0) continue;
                        const bool diagonal = dx != 0 && dy != 0;
                        if (diagonal && request.movement.connectivity == Connectivity::Four) continue;
                        const int nx = x + dx;
                        const int ny = y + dy;
                        if (nx < 0 || ny < 0 || nx >= width || ny >= height) continue;
                        const auto next = map.node_at(nx, ny);
                        if (map.blocked(next)) continue;
                        if (diagonal && request.movement.corners == CornerPolicy::ForbidCutting &&
                            (map.blocked(map.node_at(nx, y)) || map.blocked(map.node_at(x, ny)))) continue;
                        const Cost edge = (diagonal ? request.movement.diagonal_cost : request.movement.orthogonal_cost)
                            * map.terrain_weight(next);
                        if (distances[node] + edge < distances[next]) {
                            distances[next] = distances[node] + edge;
                            changed = true;
                        }
                    }
                }
            }
        }
        if (!changed) break;
    }
    return distances[request.goal];
}

void require_valid_result(const GridMap& map, const SearchRequest& request, const SearchResult& result) {
    REQUIRE(result.status == SearchStatus::Success);
    const auto validation = validate_path(map, request.movement, request.start, request.goal, result.path);
    REQUIRE(validation.valid);
    REQUIRE(validation.cost == result.cost);
}
} // namespace

TEST_CASE("All searches solve analytic corridors and same-node requests", "[search]") {
    for (const auto algorithm : algorithms) {
        INFO(to_string(algorithm));
        GridMap map(6, 1);
        auto request = request_for(map, algorithm);
        auto result = search(map, request);
        require_valid_result(map, request, result);
        REQUIRE(result.cost == 5000);
        REQUIRE(result.path.size() == 6);
        REQUIRE(result.optimal_cost_guaranteed == (algorithm != Algorithm::Dfs));
        request.goal = request.start;
        result = search(map, request);
        require_valid_result(map, request, result);
        REQUIRE(result.cost == 0);
        REQUIRE(result.path == std::vector<NodeId>{0});
    }
}

TEST_CASE("Disconnected maps and invalid endpoints have different statuses", "[search]") {
    for (const auto algorithm : algorithms) {
        GridMap map(3, 1);
        map.set_blocked(1, true);
        auto request = request_for(map, algorithm);
        const auto result = search(map, request);
        REQUIRE(result.status == SearchStatus::NoPath);
        REQUIRE(result.path.empty());
        REQUIRE(result.cost == infinite_cost);
        request.start = invalid_node;
        REQUIRE(search(map, request).status == SearchStatus::InvalidRequest);
        request.start = 1;
        REQUIRE(search(map, request).status == SearchStatus::InvalidRequest);
        request.start = 0;
        request.goal = 99;
        REQUIRE(search(map, request).status == SearchStatus::InvalidRequest);
    }
}

TEST_CASE("Weighted searches take a cheaper detour", "[search][weighted]") {
    GridMap map(5, 3);
    for (NodeId node = 6; node <= 8; ++node) map.set_terrain_weight(node, 9);
    for (const auto algorithm : {Algorithm::Dijkstra, Algorithm::AStar}) {
        auto request = request_for(map, algorithm);
        request.start = 5;
        request.goal = 9;
        const auto result = search(map, request);
        require_valid_result(map, request, result);
        REQUIRE(result.cost == 6000);
        REQUIRE(result.cost == oracle(map, request));
    }
    auto request = request_for(map, Algorithm::Bfs);
    REQUIRE(search(map, request).status == SearchStatus::UnsupportedConfiguration);
    request.algorithm = Algorithm::Dfs;
    require_valid_result(map, request, search(map, request));
}

TEST_CASE("Dijkstra improves a goal already discovered through an expensive diagonal", "[search][regression]") {
    GridMap map(2, 2);
    map.set_terrain_weight(3, 3);
    auto request = request_for(map, Algorithm::Dijkstra);
    request.movement.connectivity = Connectivity::Eight;
    const auto result = search(map, request);
    require_valid_result(map, request, result);
    REQUIRE(result.cost == 4000);
    REQUIRE(result.path.size() == 3);
    REQUIRE(result.metrics.relaxations > result.metrics.discovered_unique - 1);
}

TEST_CASE("Diagonal heuristics and corner rules agree with the model", "[search][diagonal]") {
    GridMap map(4, 4);
    auto request = request_for(map, Algorithm::AStar);
    request.movement.connectivity = Connectivity::Eight;
    REQUIRE(search(map, request).cost == 4242);
    request.movement.diagonal_cost = 1000;
    REQUIRE(search(map, request).cost == 3000);
    request.algorithm = Algorithm::Bfs;
    REQUIRE(search(map, request).cost == 3000);
    request.movement.diagonal_cost = 1414;
    REQUIRE(search(map, request).status == SearchStatus::UnsupportedConfiguration);

    GridMap blocked(2, 2);
    blocked.set_blocked(1, true);
    blocked.set_blocked(2, true);
    request = request_for(blocked, Algorithm::AStar);
    request.movement.connectivity = Connectivity::Eight;
    REQUIRE(search(blocked, request).status == SearchStatus::NoPath);
    request.movement.corners = CornerPolicy::AllowCutting;
    REQUIRE(search(blocked, request).cost == 1414);
}

TEST_CASE("Sessions own their snapshots and can advance independently", "[search][session]") {
    GridMap map(12, 8);
    auto request = request_for(map, Algorithm::AStar);
    SearchSession first(map, request);
    SearchSession second(map, request);
    map.set_blocked(request.goal, true);
    REQUIRE(first.step(0) == SearchStatus::Running);
    REQUIRE(first.result().metrics.expanded_total == 0);
    first.step(1);
    second.step(7);
    first.cancel();
    first.cancel();
    REQUIRE(first.step(1000) == SearchStatus::Cancelled);
    REQUIRE(first.result().path.empty());
    while (second.step(3) == SearchStatus::Running) {}
    REQUIRE(second.result().status == SearchStatus::Success);
    REQUIRE(second.result().cost == 18000);
    second.cancel();
    REQUIRE(second.result().status == SearchStatus::Success);
    REQUIRE(search(map, request).status == SearchStatus::InvalidRequest);
}

TEST_CASE("Step sizes preserve paths and expansion order", "[search][session]") {
    GridMap map(8, 6);
    map.set_blocked(map.node_at(3, 0), true);
    map.set_terrain_weight(map.node_at(2, 2), 8);
    for (const auto algorithm : {Algorithm::Dijkstra, Algorithm::AStar}) {
        const auto request = request_for(map, algorithm);
        const auto expected = search(map, request);
        for (const std::size_t budget : {1, 2, 7, 256}) {
            SearchSession session(map, request);
            while (session.step(budget) == SearchStatus::Running) {}
            REQUIRE(session.result().cost == expected.cost);
            REQUIRE(session.result().path == expected.path);
            REQUIRE(session.result().expanded_order == expected.expanded_order);
        }
    }
}

TEST_CASE("Limits are not confused with proof of no path", "[search][limits]") {
    GridMap map(10, 10);
    auto request = request_for(map, Algorithm::Dijkstra);
    request.max_expansions = 1;
    const auto limited = search(map, request);
    REQUIRE(limited.status == SearchStatus::ResourceLimit);
    REQUIRE(limited.metrics.expanded_total == 1);
    REQUIRE(limited.path.empty());
    request.max_expansions = 0;
    request.record_expansions = false;
    const auto result = search(map, request);
    require_valid_result(map, request, result);
    REQUIRE(result.expanded_order.empty());
    REQUIRE(result.metrics.expanded_total > 0);
}

TEST_CASE("Costs remain exact above the 32-bit range", "[search][weighted]") {
    GridMap map(20, 1);
    for (NodeId node = 0; node < map.size(); ++node) map.set_terrain_weight(node, 1000);
    auto request = request_for(map, Algorithm::AStar);
    request.movement.orthogonal_cost = 1000000;
    const auto result = search(map, request);
    require_valid_result(map, request, result);
    REQUIRE(result.cost == 19000000000ULL);
}

TEST_CASE("Unknown configurations are refused", "[search][validation]") {
    GridMap map(2, 2);
    auto request = request_for(map, Algorithm::AStar);
    request.algorithm = static_cast<Algorithm>(99);
    REQUIRE(search(map, request).status == SearchStatus::UnsupportedConfiguration);
    request.algorithm = Algorithm::AStar;
    request.heuristic = static_cast<Heuristic>(99);
    REQUIRE(search(map, request).status == SearchStatus::UnsupportedConfiguration);
    request.heuristic = Heuristic::Automatic;
    request.movement.orthogonal_cost = 0;
    REQUIRE(search(map, request).status == SearchStatus::InvalidRequest);
}

TEST_CASE("A star and Dijkstra agree with an independent oracle on generated maps", "[search][oracle]") {
    std::uint32_t seed = 0x51a7e123;
    auto next_random = [&]() { seed = seed * 1664525U + 1013904223U; return seed; };
    for (int sample = 0; sample < 80; ++sample) {
        const auto width = 3 + next_random() % 5;
        const auto height = 3 + next_random() % 5;
        GridMap map(width, height);
        const NodeId goal = static_cast<NodeId>(map.size() - 1);
        for (NodeId node = 0; node < map.size(); ++node) {
            if (node != 0 && node != goal) map.set_blocked(node, next_random() % 5 == 0);
            map.set_terrain_weight(node, 1 + next_random() % 9);
        }
        for (const auto connectivity : {Connectivity::Four, Connectivity::Eight}) {
            for (const auto corners : {CornerPolicy::ForbidCutting, CornerPolicy::AllowCutting}) {
                for (const auto algorithm : {Algorithm::Dijkstra, Algorithm::AStar}) {
                    auto request = request_for(map, algorithm);
                    request.movement.connectivity = connectivity;
                    request.movement.corners = corners;
                    const auto expected = oracle(map, request);
                    const auto result = search(map, request);
                    INFO("sample=" << sample << " seed=" << seed << " algorithm=" << to_string(algorithm));
                    REQUIRE(result.cost == expected);
                    if (expected == infinite_cost) REQUIRE(result.status == SearchStatus::NoPath);
                    else require_valid_result(map, request, result);
                    request.heuristic = Heuristic::Zero;
                    REQUIRE(search(map, request).cost == expected);
                }
            }
        }
    }
}
