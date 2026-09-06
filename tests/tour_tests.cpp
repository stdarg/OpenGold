#include "opengold/rolf_tour.h"
#include "opengold/exploration_view.h"
#include <algorithm>
#include <cstdlib>
#include <iostream>

using namespace opengold::por;
namespace {
using Bytes = std::vector<std::uint8_t>;
void check(bool ok, const char* message) { if (!ok) throw std::runtime_error(message); }
std::shared_ptr<const EclProgram> program(Bytes body)
{
    Bytes record{0, 0};
    for (int n = 0; n < 5; ++n) record.insert(record.end(), {1, 1, 0x14, 0x99});
    record.insert(record.end(), body.begin(), body.end());
    return std::make_shared<const EclProgram>(EclProgram::decode(record, "synthetic tour"));
}
void step_to_prompt(RolfTourSession& tour)
{
    for (int n = 0; n < 500 && tour.snapshot().phase == TourPhase::running; ++n) tour.advance(0.3);
    check(tour.snapshot().phase != TourPhase::faulted, tour.snapshot().diagnostic.c_str());
    check(tour.snapshot().phase != TourPhase::running, "Host must yield a prompt or finish");
}
void wall_art_tests()
{
    Bytes tiles_record(17 + 4 * 32, 0);
    tiles_record[0] = 8; tiles_record[2] = 1; tiles_record[8] = 4;
    std::fill_n(tiles_record.begin() + 17, 32, 0x11);
    std::fill_n(tiles_record.begin() + 49, 32, 0x88);
    std::fill_n(tiles_record.begin() + 113, 32, 0xdd);
    auto tiles = decode_wall_tiles(tiles_record);
    check(tiles && tiles->size() == 4 && (*tiles)[1][63] == 8, "Decode normal 8x8 tile frames");
    tiles_record.pop_back();
    check(!decode_wall_tiles(tiles_record), "Reject truncated tile record");
    tiles_record.push_back(0xdd); tiles_record[2] = 2;
    check(!decode_wall_tiles(tiles_record), "Reject unsupported tile dimensions");

    Bytes definitions(156 * 2, 1);
    std::fill(definitions.begin() + 156, definitions.end(), 2);
    auto art = decode_wall_art(definitions, *tiles);
    check(art && art->appearances.size() == 2, "Two appearances with ten views each");
    const auto& face = art->appearances[0][6];
    check(face.width == 56 && face.height == 64, "Near-facing art retains authored dimensions");
    check(face.rgba[0] == 85 && face.rgba[3] == 255, "Wall palette index eight is opaque gray");
    check(art->appearances[1][6].rgba[0] == 0 && art->appearances[1][6].rgba[3] == 255, "Black is opaque in wall art");
    definitions[54] = 3; definitions[55] = 0;
    const auto cutouts = decode_wall_art(definitions, *tiles);
    check(cutouts && cutouts->appearances[0][6].rgba[3] == 0 &&
        cutouts->appearances[0][6].rgba[8 * 4 + 3] == 0, "Pink and tile zero reveal the backdrop");
    definitions[55] = 4;
    check(!decode_wall_art(definitions, *tiles), "Out-of-bank tile reference rejected");
    definitions.pop_back();
    check(!decode_wall_art(definitions, *tiles), "Partial appearance rejected");

    GeoMap map;
    map.cells[8 * 16 + 8].walls[0] = 1;
    map.cells[7 * 16 + 8].walls[2] = 2;
    map.cells[7 * 16 + 8].walls[0] = 2;
    auto view = compose_exploration_view(map, *art, 8, 8, 0);
    check(view.width == 88 && view.height == 88 && view.rgba[(40 * 88 + 44) * 4] == 85,
        "Near face hides farther opaque artwork");
    check(view.rgba[(8 * 88 + 16) * 4] == 85 && view.rgba[(71 * 88 + 71) * 4] == 85,
        "Full near face fits inside the frame");
    view = compose_exploration_view(map, *art, 8, 7, 2);
    check(view.rgba[(40 * 88 + 44) * 4] == 0, "Opposite edge keeps its distinct artwork");
    for (unsigned facing = 0; facing < 4; ++facing) {
        GeoMap rotated;
        rotated.cells[8 * 16 + 8].walls[facing] = 1;
        const auto image = compose_exploration_view(rotated, *art, 8, 8, facing);
        check(image.rgba[(40 * 88 + 44) * 4] == 85, "Directional wall ID follows every facing");
    }
    GeoMap empty;
    const auto background = compose_exploration_view(empty, *art, 0, 0, 3);
    empty.cells[0].doors[3] = 1;
    check(compose_exploration_view(empty, *art, 0, 0, 3).rgba == background.rgba,
        "Door interaction bits do not fabricate door artwork at map boundaries");
}
void synthetic()
{
    const auto p = program({
        9,0,3,1,0x4b,0xc0, 9,0,4,1,0x4c,0xc0, 9,0,1,1,0x4d,0xc0,
        45,1,0x90,0x2c, 12,0,12,0,2,0,9, 58,13,58,13,
        18,128,3,4,32,192, // Generated "ABC" text.
        43,1,1,0x98,0,1,128,3,4,32,192,
        49,9,0,4,1,0x4b,0xc0,45,1,0x90,0x2c,0});
    EclMachine vm(p);
    check(!vm.start_at_for_inspection(0x9901), "Reject entry table interior");
    check(vm.state() == EclState::idle, "Invalid inspection start does not mutate state");
    check(vm.start_at_for_inspection(0x9914), "Explicit isolated start");
    check(!vm.start_at_for_inspection(0x9914), "Cannot replace a running invocation");
    GeoMap map;
    map.cells[4 * 16 + 3].event_raw = 7;
    map.cells[4 * 16 + 4].walls[1] = 1;
    map.cells[3 * 16 + 4].doors[0] = 1;
    RolfTourSession tour(map, p, {}, 0x9914);
    check(!tour.explore(ExplorationCommand::forward), "Movement locked while scripts run");
    tour.advance(0);
    check(tour.snapshot().sprite_frame == 2, "Far sprite setup and delay");
    check(tour.script_variable(0xC04F) == 7, "Redraw derives current map event");
    tour.advance(0); check(tour.snapshot().sprite_frame == 2, "Delay is nonblocking and not reissued");
    step_to_prompt(tour);
    check(tour.snapshot().pose == PartyPose{3,4,1}, "Script controls party pose");
    check(tour.snapshot().sprite_frame == 0 && tour.snapshot().dialogue == "ABC", "Near sprite and decoded text");
    const auto ticket = tour.snapshot().continue_ticket;
    check(!tour.continue_dialogue(ticket+1), "Wrong ticket rejected");
    check(tour.continue_dialogue(ticket) && !tour.continue_dialogue(ticket), "Reply applied once");
    step_to_prompt(tour);
    check(tour.snapshot().phase == TourPhase::completed && tour.snapshot().sprite_frame == -1, "Tour completes and hides encounter");
    check(tour.snapshot().pose == PartyPose{4,4,1}, "Later scripted redraw");
    check(!tour.explore(ExplorationCommand::forward), "Wall blocks exploration");
    check(tour.explore(ExplorationCommand::turn_left), "Turn after completion");
    check(tour.script_variable(0xC04D) == 0, "Exploration and VM share authoritative pose");
    check(tour.explore(ExplorationCommand::forward), "Open edge permits a step");
    check(!tour.explore(ExplorationCommand::forward), "Door blocks inspection movement");
    tour.explore(ExplorationCommand::turn_right);
    check(tour.explore(ExplorationCommand::forward), "Eastward open edge permits a step");
    tour.explore(ExplorationCommand::turn_right);
    check(tour.explore(ExplorationCommand::forward), "Southward open edge permits a step");
    tour.explore(ExplorationCommand::turn_right);
    check(!tour.explore(ExplorationCommand::forward), "Neighbor wall blocks westward movement");
    tour.explore(ExplorationCommand::turn_right);
    for (unsigned i=0;i<4;++i) check(tour.explore(ExplorationCommand::forward), "Northward steps stay in the map");
    check(!tour.explore(ExplorationCommand::forward), "Boundary prevents wrapping");
    tour.restart(); step_to_prompt(tour);
    check(!tour.continue_dialogue(ticket), "Stale pre-restart ticket rejected");
    const auto bad = program({45,1,0,0x80,0});
    RolfTourSession unsupported({},bad,{},0x9914); unsupported.advance(0);
    check(unsupported.snapshot().phase == TourPhase::faulted, "Unsupported services stop with diagnostics");
}
void installed(const char* directory)
{
    auto tour = RolfTourSession::load(directory);
    check(tour.wall_art().appearances.size() == 15, "Load all fifteen Phlan appearances");
    check(tour.wall_art().appearances[5][6].rgba != tour.wall_art().appearances[7][6].rgba,
        "Distinct stone masonry patterns retained");
    check(tour.map().at(4,3).walls[2] == 11 && tour.map().at(5,2).walls[1] == 12 &&
        tour.map().at(11,2).walls[2] == 13, "Original landmark IDs select city hall, training hall and temple");
    unsigned prompts = 0;
    while (true) {
        step_to_prompt(tour);
        if (tour.snapshot().phase == TourPhase::completed) break;
        const auto& s = tour.snapshot();
        ++prompts;
        std::cout << "Prompt " << prompts << " at " << s.pose.x << ',' << s.pose.y << " facing " << s.pose.facing << '\n';
        check(prompts <= 16, "Bounded installed tour");
        check(!s.dialogue.empty(), "Original dialogue shown at each pause");
        const auto image = compose_exploration_view(tour.map(), tour.wall_art(), s.pose.x, s.pose.y, s.pose.facing);
        check(image.rgba.size() == 88 * 88 * 4, "Every tour pause composes original artwork");
        check(!tour.explore(ExplorationCommand::forward), "No movement during original dialogue");
        check(tour.continue_dialogue(s.continue_ticket), "Original Continue accepted");
    }
    check(prompts == 8, "Welcome, six landmarks, farewell");
    check(tour.snapshot().redraws > 20 && tour.snapshot().footsteps > 20, "Tour extends across original table-driven route");
    check(tour.script_variable(0x4AC5) == 1, "Original tour flag retained");
    std::cout << "Installed tour complete: " << prompts << " prompts, " << tour.snapshot().redraws << " redraws.\n";
}
}
int main()
{
    try {
        wall_art_tests(); synthetic();
        if (const auto directory = std::getenv("OPENGOLD_GAME_DIR")) installed(directory);
        std::cout << "Tour tests passed.\n";
    } catch (const std::exception& error) { std::cerr << error.what() << '\n'; return 1; }
}
