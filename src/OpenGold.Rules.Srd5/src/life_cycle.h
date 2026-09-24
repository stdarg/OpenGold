#ifndef OPENGOLD_SRD5_LIFE_CYCLE_H
#define OPENGOLD_SRD5_LIFE_CYCLE_H
#include <cstdint>

namespace opengold::srd5::detail {
inline constexpr unsigned death_turn_ms=6000;
inline constexpr unsigned recovery_hour_ms=3600000;
struct RecoveryClock {
    unsigned death_save_in_ms{}, stable_recovery_in_ms{};
    bool operator==(const RecoveryClock&) const = default;
};
struct LifeState {
    int hp{}, successes{}, failures{};
    bool stable{}, dead{};
    RecoveryClock recovery;
    bool operator==(const LifeState&) const = default;
};
void validate_recovery(const LifeState& state);
// Earlier saves have no timing history. Initialization never rolls or invents
// elapsed time. A Stable delay of zero means its one recovery roll is pending.
void initialize_legacy_recovery(LifeState& state);
void stabilize(LifeState& state,std::uint64_t& rng);
[[nodiscard]] int death_save(LifeState& state,std::uint64_t& rng);
void damage_life(LifeState& state,int amount,int maximum_hp,bool critical=false,bool dies_at_zero=false);
[[nodiscard]] int heal_life(LifeState& state,int amount,int maximum_hp);
void start_stable_recovery(LifeState& state,std::uint64_t& rng);
// Combat rolls death saves at turn entry; this only retains its remaining
// cadence and advances natural recovery. Campaign event scheduling is separate.
[[nodiscard]] bool advance_recovery_clock(LifeState& state,std::uint64_t milliseconds);
}
#endif
