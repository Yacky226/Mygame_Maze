#include "maze/generator.hpp"
#include <charconv>
#include <iostream>
#include <stdexcept>
#include <string_view>

namespace {
std::uint32_t number(std::string_view text) {
    std::uint32_t value = 0;
    const auto parsed = std::from_chars(text.data(), text.data() + text.size(), value);
    if (parsed.ec != std::errc{} || parsed.ptr != text.data() + text.size())
        throw std::invalid_argument("Expected an unsigned 32-bit integer");
    return value;
}
}
int main(int argc, char** argv) {
    if (argc == 2 && std::string_view(argv[1]) == "--help") {
        std::cout << "Usage: maze_generate [--generator backtracker|prim|kruskal] [--seed N] [--width N] [--height N]\n"
                     "Writes an ASCII map for maze_cli. Odd dimensions >= 3. Generator version: 1.\n"
                     "Exit: 0 success; 2 invalid input; 4 memory limit; 5 internal error.\n";
        return 0;
    }
    try {
        maze::GenerationRequest request;
        for (int i = 1; i < argc; ++i) {
            const std::string_view option = argv[i];
            if (++i >= argc) throw std::invalid_argument("Missing option value");
            const std::string_view value = argv[i];
            if (option == "--seed") request.seed = number(value);
            else if (option == "--width") request.width = number(value);
            else if (option == "--height") request.height = number(value);
            else if (option == "--generator") {
                if (value == "backtracker") request.algorithm = maze::Generator::Backtracker;
                else if (value == "prim") request.algorithm = maze::Generator::Prim;
                else if (value == "kruskal") request.algorithm = maze::Generator::Kruskal;
                else throw std::invalid_argument("Unknown generator");
            } else throw std::invalid_argument("Unknown option");
        }
        const auto generated = maze::generate_maze(request);
        for (std::uint32_t y = 0; y < generated.map.height(); ++y) {
            for (std::uint32_t x = 0; x < generated.map.width(); ++x) {
                const auto node = generated.map.node_at(x, y);
                std::cout << (node == generated.start && node == generated.goal ? 'B' :
                    node == generated.start ? 'S' : node == generated.goal ? 'G' :
                    generated.map.blocked(node) ? '#' : '.');
            }
            std::cout << '\n';
        }
        return 0;
    } catch (const std::invalid_argument& exception) {
        std::cerr << exception.what() << '\n'; return 2;
    } catch (const std::bad_alloc&) {
        std::cerr << "Out of memory\n"; return 4;
    } catch (const std::exception& exception) {
        std::cerr << exception.what() << '\n'; return 5;
    }
}
