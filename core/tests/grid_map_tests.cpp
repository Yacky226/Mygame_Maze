#include "maze/grid_map.hpp"

#include <catch2/catch_test_macros.hpp>
#include <stdexcept>

using namespace maze;

TEST_CASE("Map dimensions and coordinates are validated", "[map]") {
    REQUIRE_THROWS_AS(GridMap(0, 1), std::invalid_argument);
    REQUIRE_THROWS_AS(GridMap(1, 0), std::invalid_argument);
    REQUIRE_THROWS_AS(GridMap(65536, 65536), std::invalid_argument);
    REQUIRE_THROWS_AS(GridMap(1025, 1024), std::invalid_argument);
    GridMap map(4, 3);
    REQUIRE(map.size() == 12);
    REQUIRE(map.node_at(2, 1) == 6);
    REQUIRE(map.position(6) == Position{2, 1});
    REQUIRE_FALSE(map.contains(invalid_node));
    REQUIRE_THROWS_AS(map.node_at(4, 0), std::out_of_range);
    REQUIRE_THROWS_AS(map.position(12), std::out_of_range);
    REQUIRE_THROWS_AS(map.set_blocked(12, true), std::out_of_range);
    REQUIRE_THROWS_AS(map.set_terrain_weight(12, 1), std::out_of_range);
    REQUIRE_THROWS_AS(map.set_terrain_weight(0, 0), std::invalid_argument);
    REQUIRE_THROWS_AS(map.set_terrain_weight(0, 1001), std::invalid_argument);
}

TEST_CASE("Neighbors preserve order and use destination weights", "[map]") {
    GridMap map(4, 3);
    map.set_terrain_weight(7, 3);
    auto neighbors = map.neighbors(6, {});
    REQUIRE(neighbors.count == 4);
    REQUIRE(neighbors.values[0].node == 5);
    REQUIRE(neighbors.values[1].node == 7);
    REQUIRE(neighbors.values[1].cost == 3000);
    REQUIRE(neighbors.values[2].node == 2);
    REQUIRE(neighbors.values[3].node == 10);
    const auto reverse = map.neighbors(7, {});
    REQUIRE(reverse.values[0].node == 6);
    REQUIRE(reverse.values[0].cost == 1000);
    map.set_blocked(6, true);
    REQUIRE(map.neighbors(6, {}).count == 0);
    REQUIRE(map.neighbors(7, {}).count == 2);
}

TEST_CASE("Diagonal transitions obey the corner policy", "[map][diagonal]") {
    GridMap map(2, 2);
    map.set_blocked(1, true);
    MovementRules rules;
    rules.connectivity = Connectivity::Eight;
    REQUIRE(map.neighbors(0, rules).count == 1);
    rules.corners = CornerPolicy::AllowCutting;
    const auto neighbors = map.neighbors(0, rules);
    REQUIRE(neighbors.count == 2);
    REQUIRE(neighbors.values[1].node == 3);
    REQUIRE(neighbors.values[1].cost == 1414);
}

TEST_CASE("Movement costs are bounded and diagonal models are validated", "[map]") {
    MovementRules rules;
    rules.orthogonal_cost = 0;
    REQUIRE_FALSE(valid_rules(rules));
    rules.orthogonal_cost = 1000;
    rules.connectivity = Connectivity::Eight;
    rules.diagonal_cost = 999;
    REQUIRE_FALSE(valid_rules(rules));
    rules.diagonal_cost = 2001;
    REQUIRE_FALSE(valid_rules(rules));
    rules.diagonal_cost = 1000;
    REQUIRE(valid_rules(rules));
    rules.corners = static_cast<CornerPolicy>(99);
    REQUIRE_FALSE(valid_rules(rules));
    REQUIRE(add_cost(infinite_cost - 1, 2) == infinite_cost);
    REQUIRE(add_cost(infinite_cost, 1) == infinite_cost);
}

TEST_CASE("Path validation rejects invalid nodes and transitions", "[map][validation]") {
    GridMap map(3, 2);
    const std::vector<NodeId> correct{0, 1, 2};
    REQUIRE(validate_path(map, {}, 0, 2, correct).cost == 2000);
    const std::vector<NodeId> jump{0, 2};
    REQUIRE_FALSE(validate_path(map, {}, 0, 2, jump).valid);
    const std::vector<NodeId> outside{0, 99, 2};
    REQUIRE_FALSE(validate_path(map, {}, 0, 2, outside).valid);
    REQUIRE_FALSE(validate_path(map, {}, 1, 2, correct).valid);
    REQUIRE_FALSE(validate_path(map, {}, 0, 2, {}).valid);
    map.set_blocked(1, true);
    REQUIRE_FALSE(validate_path(map, {}, 0, 2, correct).valid);
}
