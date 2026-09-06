#include "opengold/rolf_tour.h"
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
    unsigned prompts = 0;
    while (true) {
        step_to_prompt(tour);
        if (tour.snapshot().phase == TourPhase::completed) break;
        const auto& s = tour.snapshot();
        ++prompts;
        std::cout << "Prompt " << prompts << " at " << s.pose.x << ',' << s.pose.y << " facing " << s.pose.facing << '\n';
        check(prompts <= 16, "Bounded installed tour");
        check(!s.dialogue.empty(), "Original dialogue shown at each pause");
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
        synthetic();
        if (const auto directory = std::getenv("OPENGOLD_GAME_DIR")) installed(directory);
        std::cout << "Tour tests passed.\n";
    } catch (const std::exception& error) { std::cerr << error.what() << '\n'; return 1; }
}
