#ifndef OPENGOLD_ROLF_TOUR_H
#define OPENGOLD_ROLF_TOUR_H

#include "opengold/ecl_machine.h"
#include "opengold/map_catalog.h"
#include "opengold/formats.h"
#include <bitset>

namespace opengold::por {
struct PartyPose {
    unsigned x{}, y{}, facing{}; // GEO coordinates, 0=N, 1=E, 2=S, 3=W.
    auto operator<=>(const PartyPose&) const = default;
};
enum class TourPhase { running, awaiting_continue, completed, faulted };
struct TourSnapshot {
    PartyPose pose;
    TourPhase phase{TourPhase::running};
    std::string dialogue, diagnostic;
    std::uint64_t revision{}, continue_ticket{};
    unsigned prompts{}, redraws{}, footsteps{};
    int sprite_frame{-1}; // -1 hidden; native near/medium/far frames are 0/1/2.
    std::bitset<256> visited;
};
enum class ExplorationCommand { turn_left, turn_right, forward };

// Isolated tour host, not a campaign scheduler. Original bytecode supplies the
// dialogue and route. Bound VM position cells are authoritative; snapshot_ is a
// read-only presentation cache published at redraw/request boundaries.
class RolfTourSession {
public:
    [[nodiscard]] static RolfTourSession load(const std::filesystem::path& directory);
    // Also accepts wholly synthetic resources for asset-free host tests.
    RolfTourSession(GeoMap map, std::shared_ptr<const EclProgram> program,
                   std::array<opengold::Image, 3> sprites, std::uint32_t entry);
    void restart();
    void advance(double seconds);
    bool continue_dialogue(std::uint64_t ticket);
    bool explore(ExplorationCommand command);
    [[nodiscard]] const TourSnapshot& snapshot() const noexcept { return snapshot_; }
    [[nodiscard]] const GeoMap& map() const noexcept { return map_; }
    [[nodiscard]] const auto& sprites() const noexcept { return sprites_; }
    [[nodiscard]] std::uint16_t script_variable(std::uint16_t address) const { return machine_.variable(address); }
private:
    GeoMap map_;
    std::shared_ptr<const EclProgram> program_;
    std::array<opengold::Image, 3> sprites_;
    EclMachine machine_;
    std::uint32_t entry_;
    TourSnapshot snapshot_;
    std::uint64_t next_ticket_{}, menu_request_{}, delayed_request_{};
    double remaining_delay_{};
    void publish_pose();
    void handle_host(const EclRequest& request);
    void fail(std::string diagnostic);
};
}
#endif
