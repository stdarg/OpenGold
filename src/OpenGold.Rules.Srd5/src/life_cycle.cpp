#include "life_cycle.h"
#include "dice.h"
#include <algorithm>
#include <stdexcept>

namespace opengold::srd5::detail {
void validate_recovery(const LifeState& state)
{
    const auto& clock=state.recovery;
    if(clock.death_save_in_ms>death_turn_ms||clock.stable_recovery_in_ms>4*recovery_hour_ms||
        ((state.hp>0||state.dead)&&(clock.death_save_in_ms||clock.stable_recovery_in_ms))||
        (state.stable&&clock.death_save_in_ms)||(!state.stable&&clock.stable_recovery_in_ms))
        throw std::runtime_error("Invalid recovery clock");
}
void initialize_legacy_recovery(LifeState& state)
{
    state.recovery={};
    if(state.hp==0&&!state.dead&&!state.stable)state.recovery.death_save_in_ms=death_turn_ms;
}
void start_stable_recovery(LifeState& state,std::uint64_t& rng)
{
    if(state.hp==0&&!state.dead&&state.stable&&!state.recovery.stable_recovery_in_ms)
        state.recovery.stable_recovery_in_ms=unsigned(roll_die(rng,4))*recovery_hour_ms;
}
void stabilize(LifeState& state,std::uint64_t& rng)
{
    if(state.hp!=0||state.dead)throw std::runtime_error("Stabilization requires a living creature at zero HP");
    state.stable=true;state.successes=state.failures=0;state.recovery.death_save_in_ms=0;
    start_stable_recovery(state,rng);
}
int death_save(LifeState& state,std::uint64_t& rng)
{
    if(state.hp!=0||state.dead||state.stable)throw std::runtime_error("Death save requires an unstable living creature at zero HP");
    const int natural=roll_die(rng,20);state.recovery.death_save_in_ms=death_turn_ms;
    if(natural==20){state.hp=1;state.successes=state.failures=0;state.recovery={};}
    else if(natural>=10)++state.successes;
    else state.failures+=natural==1?2:1;
    if(state.failures>=3){state.dead=true;state.recovery={};}
    else if(state.successes>=3)stabilize(state,rng);
    return natural;
}
void damage_life(LifeState& state,int amount,int maximum_hp,bool critical,bool dies_at_zero)
{
    if(amount<0||maximum_hp<1||state.hp<0||state.hp>maximum_hp)throw std::runtime_error("Invalid damage");
    if(!amount||state.dead)return;
    const bool was_zero=state.hp==0;
    const int remaining=amount-state.hp;state.hp=std::max(0,state.hp-amount);
    if(state.hp)return;
    state.stable=false;state.recovery={death_turn_ms,0};
    if(was_zero)state.failures+=critical?2:1;
    else state.successes=state.failures=0;
    if(dies_at_zero||remaining>=maximum_hp||state.failures>=3){state.dead=true;state.recovery={};}
}
int heal_life(LifeState& state,int amount,int maximum_hp)
{
    if(amount<0||maximum_hp<1||state.hp<0||state.hp>maximum_hp||state.dead)throw std::runtime_error("Invalid healing");
    const int healed=std::min(amount,maximum_hp-state.hp);
    if(healed){state.hp+=healed;state.successes=state.failures=0;state.stable=false;state.recovery={};}
    return healed;
}
bool advance_recovery_clock(LifeState& state,std::uint64_t milliseconds)
{
    auto& clock=state.recovery;
    clock.death_save_in_ms-=static_cast<unsigned>(std::min<std::uint64_t>(milliseconds,clock.death_save_in_ms));
    if(!clock.stable_recovery_in_ms)return false;
    if(milliseconds>=clock.stable_recovery_in_ms){state.hp=1;state.stable=false;state.successes=state.failures=0;clock={};return true;}
    clock.stable_recovery_in_ms-=static_cast<unsigned>(milliseconds);return false;
}
}
