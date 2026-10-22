#include "maze/search.hpp"
#include <emscripten/bind.h>
#include <cmath>
#include <memory>
#include <stdexcept>
#include <string>

namespace {
using emscripten::val;

std::uint32_t integer(const val& object, const char* key, std::uint32_t maximum) {
    const auto input = object[key];
    if (input.typeOf().as<std::string>() != "number") throw std::invalid_argument("Expected numeric field");
    const double value = input.as<double>();
    if (!std::isfinite(value) || value < 0 || value > maximum || std::floor(value) != value)
        throw std::invalid_argument("Integer field outside supported range");
    return static_cast<std::uint32_t>(value);
}

bool boolean(const val& object, const char* key) {
    if (object[key].typeOf().as<std::string>() != "boolean") throw std::invalid_argument("Expected boolean field");
    return object[key].as<bool>();
}

val error(const std::string& message) {
    auto output = val::object();
    output.set("status", std::string("invalid_request"));
    output.set("message", message);
    return output;
}

val nodes(const std::vector<maze::NodeId>& values, std::size_t begin = 0) {
    auto output = val::array();
    for (std::size_t i = begin; i < values.size(); ++i) output.call<void>("push", values[i]);
    return output;
}

// One owner per Worker. No SFML, DOM, threads or JavaScript search implementation.
class Engine {
    std::unique_ptr<maze::GridMap> map_;
    std::unique_ptr<maze::SearchSession> session_;
    maze::SearchRequest request_;
    std::size_t cursor_ = 0;

    val response() {
        if (!session_) return error("No active session");
        const auto& result = session_->result();
        if (result.status == maze::SearchStatus::Success) {
            const auto check = maze::validate_path(*map_, request_.movement, request_.start, request_.goal, result.path);
            if (!check.valid || check.cost != result.cost) throw std::runtime_error("Invalid engine path or cost");
        }
        auto output = val::object();
        output.set("status", std::string(maze::to_string(result.status)));
        output.set("message", result.message);
        output.set("expanded", nodes(result.expanded_order, cursor_));
        cursor_ = result.expanded_order.size();
        output.set("path", nodes(result.path));
        output.set("cost", result.status == maze::SearchStatus::Success ? val(std::to_string(result.cost)) : val::null());
        output.set("optimalCostGuaranteed", result.optimal_cost_guaranteed);
        auto metrics = val::object();
        metrics.set("discoveredUnique", static_cast<double>(result.metrics.discovered_unique));
        metrics.set("expandedTotal", static_cast<double>(result.metrics.expanded_total));
        metrics.set("relaxations", static_cast<double>(result.metrics.relaxations));
        metrics.set("reopened", static_cast<double>(result.metrics.reopened));
        metrics.set("stalePops", static_cast<double>(result.metrics.stale_pops));
        metrics.set("frontierPeak", static_cast<double>(result.metrics.frontier_peak));
        metrics.set("computeActiveNs", std::to_string(result.metrics.compute_active_ns));
        output.set("metrics", metrics);
        return output;
    }

public:
    val start(const val& config) {
        session_.reset();
        map_.reset();
        cursor_ = 0;
        try {
            const auto width = integer(config, "width", 256);
            const auto height = integer(config, "height", 256);
            map_ = std::make_unique<maze::GridMap>(width, height);
            const auto blocked = config["blocked"];
            const auto weights = config["weights"];
            const auto is_array = val::global("Array");
            if (!is_array.call<bool>("isArray", blocked) || !is_array.call<bool>("isArray", weights) ||
                integer(blocked, "length", 65536) != map_->size() ||
                integer(weights, "length", 65536) != map_->size())
                throw std::invalid_argument("Map arrays must match dimensions");
            for (maze::NodeId node = 0; node < map_->size(); ++node) {
                if (blocked[node].typeOf().as<std::string>() != "number" ||
                    (blocked[node].as<double>() != 0 && blocked[node].as<double>() != 1))
                    throw std::invalid_argument("Obstacle values must be zero or one");
                map_->set_blocked(node, blocked[node].as<unsigned>() == 1);
                auto field = val::object();
                field.set("weight", weights[node]);
                map_->set_terrain_weight(node, integer(field, "weight", maze::GridMap::max_terrain_weight));
            }
            request_ = {};
            request_.start = integer(config, "start", static_cast<std::uint32_t>(map_->size() - 1));
            request_.goal = integer(config, "goal", static_cast<std::uint32_t>(map_->size() - 1));
            const auto algorithm = config["algorithm"].as<std::string>();
            if (algorithm == "astar") request_.algorithm = maze::Algorithm::AStar;
            else if (algorithm == "dijkstra") request_.algorithm = maze::Algorithm::Dijkstra;
            else if (algorithm == "bfs") request_.algorithm = maze::Algorithm::Bfs;
            else if (algorithm == "dfs") request_.algorithm = maze::Algorithm::Dfs;
            else throw std::invalid_argument("Unknown algorithm");
            const auto connectivity = integer(config, "connectivity", 8);
            if (connectivity != 4 && connectivity != 8) throw std::invalid_argument("Connectivity must be 4 or 8");
            request_.movement.connectivity = connectivity == 4 ? maze::Connectivity::Four : maze::Connectivity::Eight;
            request_.movement.corners = boolean(config, "allowCornerCutting")
                ? maze::CornerPolicy::AllowCutting : maze::CornerPolicy::ForbidCutting;
            request_.max_expansions = integer(config, "maxExpansions", 1000000);
            session_ = std::make_unique<maze::SearchSession>(*map_, request_);
            return response();
        } catch (const std::exception& exception) {
            session_.reset();
            return error(exception.what());
        }
    }

    val step(unsigned budget) {
        if (budget == 0 || budget > 4096) return error("Step budget must be between 1 and 4096");
        if (!session_) return error("No active session");
        session_->step(budget);
        return response();
    }

    val cancel() {
        if (session_) session_->cancel();
        return response();
    }
};
} // namespace

EMSCRIPTEN_BINDINGS(maze_studio) {
    emscripten::class_<Engine>("Engine")
        .constructor<>()
        .function("start", &Engine::start)
        .function("step", &Engine::step)
        .function("cancel", &Engine::cancel);
    emscripten::function("engineVersion", +[]() { return std::string(MAZE_ENGINE_VERSION); });
}
