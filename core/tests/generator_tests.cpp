#include "maze/generator.hpp"
#include "maze/search.hpp"
#include <catch2/catch_test_macros.hpp>
#include <algorithm>
#include <array>
#include <stdexcept>
#include <string>

using namespace maze;
namespace {
constexpr std::array generators{Generator::Backtracker, Generator::Prim, Generator::Kruskal};

// Check the carved graph directly, without the generator's room/edge helpers.
void require_tree(const GeneratedMaze& generated) {
    const auto& map = generated.map;
    const auto width = map.width(), height = map.height();
    std::size_t open = 0, degree_sum = 0;
    for (std::uint32_t y = 0; y < height; ++y) for (std::uint32_t x = 0; x < width; ++x) {
        const auto node = map.node_at(x, y);
        if (x == 0 || y == 0 || x + 1 == width || y + 1 == height || (x % 2 == 0 && y % 2 == 0))
            REQUIRE(map.blocked(node));
        if (x % 2 == 1 && y % 2 == 1) REQUIRE_FALSE(map.blocked(node));
        REQUIRE(map.terrain_weight(node) == 1);
        if (map.blocked(node)) continue;
        ++open;
        for (const auto [dx, dy] : std::array<std::pair<int, int>, 4>{{{-1,0},{1,0},{0,-1},{0,1}}}) {
            const auto nx = static_cast<int>(x) + dx, ny = static_cast<int>(y) + dy;
            if (nx >= 0 && ny >= 0 && nx < static_cast<int>(width) && ny < static_cast<int>(height) &&
                !map.blocked(map.node_at(nx, ny))) ++degree_sum;
        }
    }
    REQUIRE(open == 2 * ((width - 1) / 2) * ((height - 1) / 2) - 1);
    REQUIRE(degree_sum / 2 == open - 1);
    std::vector<int> distances(map.size(), -1);
    std::vector<NodeId> queue{generated.start};
    distances[generated.start] = 0;
    for (std::size_t head = 0; head < queue.size(); ++head) {
        const auto node = queue[head];
        const auto x = static_cast<int>(node % width), y = static_cast<int>(node / width);
        for (const auto [dx, dy] : std::array<std::pair<int, int>, 4>{{{-1,0},{1,0},{0,-1},{0,1}}}) {
            const auto nx = x + dx, ny = y + dy;
            if (nx < 0 || ny < 0 || nx >= static_cast<int>(width) || ny >= static_cast<int>(height)) continue;
            const auto next = map.node_at(nx, ny);
            if (map.blocked(next) || distances[next] != -1) continue;
            distances[next] = distances[node] + 1;
            queue.push_back(next);
        }
    }
    REQUIRE(queue.size() == open);
    REQUIRE(generated.start == map.node_at(1, 1));
    REQUIRE(distances[generated.goal] == *std::max_element(distances.begin(), distances.end()));
    SearchRequest request;
    request.start = generated.start;
    request.goal = generated.goal;
    const auto result = search(map, request);
    REQUIRE(result.status == SearchStatus::Success);
    REQUIRE(result.cost == static_cast<Cost>(distances[generated.goal]) * 1000);
    REQUIRE(validate_path(map, request.movement, request.start, request.goal, result.path).valid);
}
}

TEST_CASE("Generators produce connected acyclic mazes with closed borders", "[generator]") {
    for (const auto algorithm : generators) for (const auto seed : {0u, 1u, 42u, 4294967295u}) {
        INFO(to_string(algorithm) << " seed=" << seed);
        require_tree(generate_maze({41, 25, seed, algorithm}));
        require_tree(generate_maze({3, 3, seed, algorithm}));
        require_tree(generate_maze({3, 15, seed, algorithm}));
    }
}

TEST_CASE("Generator seeds reproduce exact maps and endpoints", "[generator][determinism]") {
    for (const auto algorithm : generators) {
        const auto first = generate_maze({21, 15, 42, algorithm});
        const auto repeat = generate_maze({21, 15, 42, algorithm});
        const auto other = generate_maze({21, 15, 43, algorithm});
        REQUIRE(first.start == repeat.start);
        REQUIRE(first.goal == repeat.goal);
        bool different = false;
        for (NodeId node = 0; node < first.map.size(); ++node) {
            REQUIRE(first.map.blocked(node) == repeat.map.blocked(node));
            different |= first.map.blocked(node) != other.map.blocked(node);
        }
        REQUIRE(different);
    }
}

TEST_CASE("Generators reject invalid dimensions and unknown algorithms", "[generator][validation]") {
    for (const auto [width, height] : std::array<std::pair<unsigned, unsigned>, 5>{{{0,5},{2,5},{4,5},{5,6},{1025,1025}}})
        REQUIRE_THROWS_AS(generate_maze({width, height, 42, Generator::Prim}), std::invalid_argument);
    REQUIRE_THROWS_AS(generate_maze({5, 5, 0, static_cast<Generator>(99)}), std::invalid_argument);
}

TEST_CASE("Version one generator maps retain their portable seed contract", "[generator][golden]") {
    const std::array expected{
        "#######\n#S#...#\n#.#.#.#\n#...#.#\n#####.#\n#G....#\n#######\n",
        "#######\n#S..#G#\n#.###.#\n#.....#\n#.#.###\n#.#...#\n#######\n",
        "#######\n#S....#\n###.###\n#.....#\n#.#.###\n#G#...#\n#######\n"
    };
    REQUIRE(generator_version == 1);
    for (std::size_t index = 0; index < generators.size(); ++index) {
        const auto generated = generate_maze({7, 7, 0, generators[index]});
        std::string actual;
        for (NodeId node = 0; node < generated.map.size(); ++node) {
            actual += node == generated.start ? 'S' : node == generated.goal ? 'G' :
                generated.map.blocked(node) ? '#' : '.';
            if ((node + 1) % 7 == 0) actual += '\n';
        }
        REQUIRE(actual == expected[index]);
    }
}
