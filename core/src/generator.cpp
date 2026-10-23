#include "maze/generator.hpp"
#include <array>
#include <limits>
#include <numeric>
#include <stdexcept>
#include <utility>

namespace maze {
namespace {
class Random {
    std::uint64_t state_;
public:
    explicit Random(std::uint32_t seed) : state_(seed) {}
    std::uint32_t next() {
        // SplitMix64, truncated to the upper 32 bits. Unsigned wrap is defined.
        auto value = (state_ += 0x9e3779b97f4a7c15ULL);
        value = (value ^ (value >> 30)) * 0xbf58476d1ce4e5b9ULL;
        value = (value ^ (value >> 27)) * 0x94d049bb133111ebULL;
        return static_cast<std::uint32_t>((value ^ (value >> 31)) >> 32);
    }
    std::uint32_t bounded(std::uint32_t bound) {
        const auto threshold = (0u - bound) % bound;
        std::uint32_t value;
        do { value = next(); } while (value < threshold);
        return value % bound;
    }
};

struct Edge { std::uint32_t from; std::uint32_t to; };
struct Adjacent {
    std::array<Edge, 4> edges{};
    std::size_t count = 0;
    auto begin() const { return edges.begin(); }
    auto end() const { return edges.begin() + static_cast<std::ptrdiff_t>(count); }
};

struct Builder {
    GridMap map;
    std::uint32_t columns;
    std::uint32_t rows;
    std::uint32_t rooms;
    Random random;

    explicit Builder(const GenerationRequest& request)
        : map(request.width, request.height), columns((request.width - 1) / 2),
          rows((request.height - 1) / 2), rooms(columns * rows), random(request.seed) {
        for (NodeId node = 0; node < map.size(); ++node) map.set_blocked(node, true);
        for (std::uint32_t room = 0; room < rooms; ++room) map.set_blocked(center(room), false);
    }
    NodeId center(std::uint32_t room) const {
        return map.node_at(2 * (room % columns) + 1, 2 * (room / columns) + 1);
    }
    Adjacent adjacent(std::uint32_t room) const {
        Adjacent result;
        const auto x = room % columns, y = room / columns;
        if (x > 0) result.edges[result.count++] = {room, room - 1};
        if (x + 1 < columns) result.edges[result.count++] = {room, room + 1};
        if (y > 0) result.edges[result.count++] = {room, room - columns};
        if (y + 1 < rows) result.edges[result.count++] = {room, room + columns};
        return result;
    }
    void carve(const Edge& edge) {
        map.set_blocked((center(edge.from) + center(edge.to)) / 2, false);
    }
    void backtrack() {
        std::vector<std::uint8_t> visited(rooms, 0);
        std::vector<std::uint32_t> stack{0};
        visited[0] = 1;
        while (!stack.empty()) {
            Adjacent choices;
            for (const auto edge : adjacent(stack.back()))
                if (!visited[edge.to]) choices.edges[choices.count++] = edge;
            if (choices.count == 0) { stack.pop_back(); continue; }
            const auto edge = choices.edges[random.bounded(static_cast<std::uint32_t>(choices.count))];
            carve(edge);
            visited[edge.to] = 1;
            stack.push_back(edge.to);
        }
    }
    void prim() {
        std::vector<std::uint8_t> visited(rooms, 0);
        std::vector<Edge> frontier;
        auto visit = [&](std::uint32_t room) {
            visited[room] = 1;
            for (const auto edge : adjacent(room)) if (!visited[edge.to]) frontier.push_back(edge);
        };
        visit(0);
        while (!frontier.empty()) {
            const auto index = random.bounded(static_cast<std::uint32_t>(frontier.size()));
            const auto edge = frontier[index];
            frontier[index] = frontier.back();
            frontier.pop_back();
            if (visited[edge.to]) continue;
            carve(edge);
            visit(edge.to);
        }
    }
    void kruskal() {
        std::vector<Edge> edges;
        for (std::uint32_t room = 0; room < rooms; ++room)
            for (const auto edge : adjacent(room)) if (edge.to > room) edges.push_back(edge);
        for (auto count = static_cast<std::uint32_t>(edges.size()); count > 1; --count)
            std::swap(edges[count - 1], edges[random.bounded(count)]);
        std::vector<std::uint32_t> parent(rooms), sizes(rooms, 1);
        std::iota(parent.begin(), parent.end(), 0u);
        auto find = [&](std::uint32_t room) {
            while (parent[room] != room) { parent[room] = parent[parent[room]]; room = parent[room]; }
            return room;
        };
        for (const auto edge : edges) {
            auto a = find(edge.from), b = find(edge.to);
            if (a == b) continue;
            if (sizes[a] < sizes[b]) std::swap(a, b);
            parent[b] = a;
            sizes[a] += sizes[b];
            carve(edge);
        }
    }
    GeneratedMaze finish() {
        const auto start = center(0);
        std::vector<std::uint32_t> distances(map.size(), std::numeric_limits<std::uint32_t>::max());
        std::vector<NodeId> queue{start};
        distances[start] = 0;
        auto goal = start;
        for (std::size_t head = 0; head < queue.size(); ++head) {
            const auto node = queue[head];
            for (const auto neighbor : map.neighbors(node, MovementRules{})) {
                if (distances[neighbor.node] != std::numeric_limits<std::uint32_t>::max()) continue;
                distances[neighbor.node] = distances[node] + 1;
                if (distances[neighbor.node] > distances[goal]) goal = neighbor.node;
                queue.push_back(neighbor.node);
            }
        }
        return {std::move(map), start, goal};
    }
};
} // namespace

GeneratedMaze generate_maze(const GenerationRequest& request) {
    if (request.width < 3 || request.height < 3 || request.width % 2 == 0 || request.height % 2 == 0)
        throw std::invalid_argument("Maze dimensions must be odd and at least 3");
    if (request.algorithm != Generator::Backtracker && request.algorithm != Generator::Prim &&
        request.algorithm != Generator::Kruskal) throw std::invalid_argument("Unknown maze generator");
    Builder builder(request);
    switch (request.algorithm) {
    case Generator::Backtracker: builder.backtrack(); break;
    case Generator::Prim: builder.prim(); break;
    case Generator::Kruskal: builder.kruskal(); break;
    }
    return builder.finish();
}

std::string_view to_string(Generator algorithm) noexcept {
    switch (algorithm) {
    case Generator::Backtracker: return "backtracker";
    case Generator::Prim: return "prim";
    case Generator::Kruskal: return "kruskal";
    }
    return "unknown";
}
} // namespace maze
