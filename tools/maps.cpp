#include "opengold/map_catalog.h"
#include <iostream>
int main(int argc, char** argv)
{
    if (argc != 2) { std::cerr << "Usage: opengold_maps GAME_DIRECTORY\n"; return 2; }
    try {
        const auto catalog = opengold::por::MapCatalog::load(argv[1]);
        std::cout << "{\"version\":1,\"maps\":[";
        bool first = true;
        for (const auto& [id, map] : catalog.all()) {
            if (!first) std::cout << ',';
            first = false;
            // Archive names are restricted to GEO + digits + .DAX by the loader.
            std::cout << "{\"archive\":\"" << id.archive << "\",\"id\":" << unsigned(id.record)
                      << ",\"bytes\":" << map.raw.size() << ",\"cells\":[";
            for (std::size_t i = 0; i < map.cells.size(); ++i) {
                if (i) std::cout << ',';
                const auto& c = map.cells[i];
                std::cout << "{\"walls\":[";
                for (unsigned d = 0; d < 4; ++d) { if (d) std::cout << ','; std::cout << unsigned(c.walls[d]); }
                std::cout << "],\"doors\":[";
                for (unsigned d = 0; d < 4; ++d) { if (d) std::cout << ','; std::cout << unsigned(c.doors[d]); }
                std::cout << "],\"event\":" << unsigned(c.event_raw) << '}';
            }
            std::cout << "]}";
        }
        std::cout << "]}\n";
        return std::cout ? 0 : 1;
    } catch (const std::exception& e) { std::cerr << e.what() << '\n'; return 1; }
}
