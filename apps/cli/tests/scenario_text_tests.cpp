#include "scenario_text.hpp"

#include <catch2/catch_test_macros.hpp>
#include <sstream>
#include <stdexcept>

TEST_CASE("Text maps preserve coordinates weights and CRLF input", "[cli][parser]") {
    std::istringstream input("S9.\r\n.#G\r\n");
    const auto scenario = maze::cli::read_text_scenario(input);
    REQUIRE(scenario.map.width() == 3);
    REQUIRE(scenario.map.height() == 2);
    REQUIRE(scenario.start == 0);
    REQUIRE(scenario.goal == 5);
    REQUIRE(scenario.map.blocked(4));
    REQUIRE(scenario.map.terrain_weight(1) == 9);
    std::istringstream same("B");
    const auto single = maze::cli::read_text_scenario(same);
    REQUIRE(single.start == single.goal);
}

TEST_CASE("Malformed text maps are refused", "[cli][parser]") {
    for (const char* text : {"", "\n", "S.\nG", "S..", "..G", "SSG", "SGG", "SB", "S?G", "S0G", "SG\n\n"}) {
        INFO(text);
        std::istringstream input(text);
        REQUIRE_THROWS_AS(maze::cli::read_text_scenario(input), std::invalid_argument);
    }
}

TEST_CASE("Text imports have a byte budget before allocation grows without bound", "[cli][parser]") {
    std::istringstream input(std::string(4 * 1024 * 1024 + 1, '.'));
    REQUIRE_THROWS_AS(maze::cli::read_text_scenario(input), std::invalid_argument);
}
