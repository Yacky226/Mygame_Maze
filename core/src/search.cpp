#include "maze/search.hpp"

#include <algorithm>
#include <chrono>
#include <deque>
#include <new>
#include <queue>
#include <utility>

namespace maze {

struct SearchSession::Impl {
    enum class State : std::uint8_t { Unseen, Open, Closed };
    struct Entry {
        NodeId node;
        Cost g;
        Cost h;
        Cost score;
        std::uint64_t sequence;
    };
    struct Compare {
        bool operator()(const Entry& a, const Entry& b) const noexcept {
            if (a.score != b.score) return a.score > b.score;
            if (a.h != b.h) return a.h > b.h;
            return a.sequence > b.sequence;
        }
    };

    GridMap map;
    SearchRequest request;
    SearchResult result;
    std::vector<Cost> distances;
    std::vector<NodeId> parents;
    std::vector<State> states;
    std::deque<NodeId> frontier;
    std::priority_queue<Entry, std::vector<Entry>, Compare> heap;
    std::uint32_t min_weight = 1;
    std::uint64_t sequence = 0;

    Impl(GridMap snapshot, SearchRequest config) : map(std::move(snapshot)), request(config) {
        if (!valid_rules(request.movement) || !map.contains(request.start) ||
            !map.contains(request.goal) || map.blocked(request.start) || map.blocked(request.goal)) {
            fail(SearchStatus::InvalidRequest, "Invalid movement rules, start or goal");
            return;
        }
        if (request.algorithm != Algorithm::Bfs && request.algorithm != Algorithm::Dfs &&
            request.algorithm != Algorithm::Dijkstra && request.algorithm != Algorithm::AStar) {
            fail(SearchStatus::UnsupportedConfiguration, "Unknown algorithm");
            return;
        }
        if (request.heuristic != Heuristic::Automatic && request.heuristic != Heuristic::Zero) {
            fail(SearchStatus::UnsupportedConfiguration, "Unknown heuristic");
            return;
        }
        if (request.algorithm == Algorithm::Bfs && !map.uniform_costs(request.movement)) {
            fail(SearchStatus::UnsupportedConfiguration, "BFS requires uniform transition costs");
            return;
        }
        try {
            distances.assign(map.size(), infinite_cost);
            parents.assign(map.size(), invalid_node);
            states.assign(map.size(), State::Unseen);
            min_weight = map.minimum_terrain_weight();
            distances[request.start] = 0;
            states[request.start] = State::Open;
            result.metrics.discovered_unique = 1;
            push(request.start);
        } catch (const std::bad_alloc&) {
            fail(SearchStatus::ResourceLimit, "Out of memory");
        }
    }

    bool weighted_frontier() const noexcept {
        return request.algorithm == Algorithm::Dijkstra || request.algorithm == Algorithm::AStar;
    }

    Cost heuristic(NodeId node) const {
        if (request.algorithm != Algorithm::AStar || request.heuristic == Heuristic::Zero) return 0;
        const auto a = map.position(node);
        const auto b = map.position(request.goal);
        const Cost dx = a.x > b.x ? a.x - b.x : b.x - a.x;
        const Cost dy = a.y > b.y ? a.y - b.y : b.y - a.y;
        if (request.movement.connectivity == Connectivity::Four) {
            return (dx + dy) * request.movement.orthogonal_cost * min_weight;
        }
        const Cost diagonal_steps = std::min(dx, dy);
        return (diagonal_steps * request.movement.diagonal_cost +
            (std::max(dx, dy) - diagonal_steps) * request.movement.orthogonal_cost) * min_weight;
    }

    void push(NodeId node) {
        if (weighted_frontier()) {
            const Cost h = heuristic(node);
            heap.push({node, distances[node], h, add_cost(distances[node], h), sequence++});
        } else {
            frontier.push_back(node);
        }
        const auto size = static_cast<std::uint64_t>(weighted_frontier() ? heap.size() : frontier.size());
        result.metrics.frontier_peak = std::max(result.metrics.frontier_peak, size);
    }

    bool empty() const noexcept { return weighted_frontier() ? heap.empty() : frontier.empty(); }

    void fail(SearchStatus status, const char* message) {
        result.status = status;
        result.cost = infinite_cost;
        result.path.clear();
        result.message = message;
    }

    void finish(NodeId node) {
        // Defensive bound: a corrupt parent chain must never hang the caller.
        for (std::size_t count = 0; count < map.size(); ++count) {
            result.path.push_back(node);
            if (node == request.start) {
                std::reverse(result.path.begin(), result.path.end());
                result.cost = distances[request.goal];
                result.status = SearchStatus::Success;
                result.optimal_cost_guaranteed = request.algorithm != Algorithm::Dfs;
                return;
            }
            node = parents[node];
            if (!map.contains(node)) break;
        }
        fail(SearchStatus::InternalError, "Invalid parent chain");
    }

    void advance() {
        NodeId current;
        if (weighted_frontier()) {
            const Entry entry = heap.top();
            heap.pop();
            current = entry.node;
            if (entry.g != distances[current] || states[current] == State::Closed) {
                ++result.metrics.stale_pops;
                return;
            }
        } else if (request.algorithm == Algorithm::Dfs) {
            current = frontier.back();
            frontier.pop_back();
        } else {
            current = frontier.front();
            frontier.pop_front();
        }

        states[current] = State::Closed;
        ++result.metrics.expanded_total;
        if (request.record_expansions) result.expanded_order.push_back(current);
        if (current == request.goal) {
            finish(current);
            return;
        }

        for (const auto neighbor : map.neighbors(current, request.movement)) {
            const bool unseen = states[neighbor.node] == State::Unseen;
            if (!weighted_frontier() && !unseen) continue;
            const Cost candidate = add_cost(distances[current], neighbor.cost);
            if (candidate >= distances[neighbor.node]) continue;
            if (unseen) ++result.metrics.discovered_unique;
            if (states[neighbor.node] == State::Closed) ++result.metrics.reopened;
            distances[neighbor.node] = candidate;
            parents[neighbor.node] = current;
            states[neighbor.node] = State::Open;
            ++result.metrics.relaxations;
            push(neighbor.node);
        }
    }
};

SearchSession::SearchSession(GridMap map, SearchRequest request)
    : impl_(std::make_unique<Impl>(std::move(map), request)) {}
SearchSession::~SearchSession() = default;
SearchSession::SearchSession(SearchSession&&) noexcept = default;
SearchSession& SearchSession::operator=(SearchSession&&) noexcept = default;

SearchStatus SearchSession::step(std::size_t max_operations) {
    if (impl_->result.status != SearchStatus::Running || max_operations == 0) {
        return impl_->result.status;
    }
    const auto started = std::chrono::steady_clock::now();
    try {
        for (std::size_t operation = 0; operation < max_operations; ++operation) {
            if (impl_->empty()) {
                impl_->result.status = SearchStatus::NoPath;
                break;
            }
            if (impl_->request.max_expansions != 0 &&
                impl_->result.metrics.expanded_total >= impl_->request.max_expansions) {
                impl_->fail(SearchStatus::ResourceLimit, "Expansion budget exhausted");
                break;
            }
            impl_->advance();
            if (impl_->result.status != SearchStatus::Running) break;
        }
        if (impl_->result.status == SearchStatus::Running && impl_->empty()) {
            impl_->result.status = SearchStatus::NoPath;
        }
    } catch (const std::bad_alloc&) {
        impl_->fail(SearchStatus::ResourceLimit, "Out of memory");
    }
    const auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::steady_clock::now() - started).count();
    impl_->result.metrics.compute_active_ns += static_cast<std::uint64_t>(elapsed);
    return impl_->result.status;
}

void SearchSession::cancel() noexcept {
    if (impl_->result.status == SearchStatus::Running) {
        impl_->result.status = SearchStatus::Cancelled;
    }
}

const SearchResult& SearchSession::result() const noexcept { return impl_->result; }

SearchResult search(GridMap map, SearchRequest request) {
    SearchSession session(std::move(map), request);
    while (session.result().status == SearchStatus::Running) session.step(4096);
    return session.result();
}

std::string_view to_string(Algorithm algorithm) noexcept {
    switch (algorithm) {
    case Algorithm::Bfs: return "bfs";
    case Algorithm::Dfs: return "dfs";
    case Algorithm::Dijkstra: return "dijkstra";
    case Algorithm::AStar: return "astar";
    }
    return "unknown";
}

std::string_view to_string(SearchStatus status) noexcept {
    switch (status) {
    case SearchStatus::Running: return "running";
    case SearchStatus::Success: return "success";
    case SearchStatus::NoPath: return "no_path";
    case SearchStatus::Cancelled: return "cancelled";
    case SearchStatus::ResourceLimit: return "resource_limit";
    case SearchStatus::InvalidRequest: return "invalid_request";
    case SearchStatus::UnsupportedConfiguration: return "unsupported_configuration";
    case SearchStatus::InternalError: return "internal_error";
    }
    return "unknown";
}

} // namespace maze
