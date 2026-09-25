#ifndef OPENGOLD_SRD5_LIFE_CYCLE_H
#define OPENGOLD_SRD5_LIFE_CYCLE_H
#include "opengold/rules.h"
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
    rules::TemporaryHitPoints temporary_hp;
    bool operator==(const LifeState&) const = default;
};
void validate_recovery(const LifeState& state);
void validate_temporary_hp(const rules::TemporaryHitPoints& pool);
void grant_temporary_hp(LifeState&,const rules::TemporaryHitPoints&,rules::TemporaryHpChoice);
// Earlier saves have no timing history. Initialization never rolls or invents
// elapsed time. A Stable delay of zero means its one recovery roll is pending.
void initialize_legacy_recovery(LifeState& state);
void stabilize(LifeState& state,std::uint64_t& rng);
[[nodiscard]] int death_save(LifeState& state,std::uint64_t& rng,bool can_heal=true);
void damage_life(LifeState& state,int amount,int maximum_hp,bool critical=false,bool dies_at_zero=false);
// Original script assignments change actual HP, bypassing the damage buffer.
void set_life_hit_points(LifeState& state,int hit_points,int maximum_hp);
[[nodiscard]] int heal_life(LifeState& state,int amount,int maximum_hp,bool can_heal=true);
void start_stable_recovery(LifeState& state,std::uint64_t& rng);
// Combat rolls death saves at turn entry; this only retains its remaining
// cadence and advances natural recovery. Campaign event scheduling is separate.
[[nodiscard]] bool advance_recovery_clock(LifeState& state,std::uint64_t milliseconds);
}
#endif
