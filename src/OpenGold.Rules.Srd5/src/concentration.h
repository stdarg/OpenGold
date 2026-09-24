#ifndef OPENGOLD_SRD5_CONCENTRATION_H
#define OPENGOLD_SRD5_CONCENTRATION_H
#include "status_effects.h"
#include <algorithm>
#include <stdexcept>
#include <utility>
namespace opengold::srd5::detail {
// Identifies an application, not just a spell: a recast must retire the old area.
struct ConcentrationSource {
    std::uint64_t scope{}, application{};
    rules::EntityId caster{};
    bool operator==(const ConcentrationSource&) const = default;
};
struct Concentration {
    ConcentrationSource source;
    std::uint64_t remaining_ms{};
    bool operator==(const Concentration&) const = default;
};
// One instance per owner. Returned sources tell the integration which effects
// to remove; this state machine never owns or reaches into combat actors.
class ConcentrationState {
public:
    const std::optional<Concentration>& active() const { return active_; }
    std::optional<ConcentrationSource> end() {
        auto old=std::exchange(active_,std::nullopt);
        return old?std::optional(old->source):std::nullopt;
    }
    std::optional<ConcentrationSource> begin(Concentration next, bool incapacitated_or_dead=false) {
        if(!next.source.scope||!next.source.application||!next.source.caster||!next.remaining_ms||incapacitated_or_dead)
            throw std::runtime_error("Invalid concentration start");
        auto old=end();active_=next;return old;
    }
    std::optional<ConcentrationSource> elapse(std::uint64_t milliseconds) {
        if(!active_)return std::nullopt;
        if(milliseconds>=active_->remaining_ms)return end();
        active_->remaining_ms-=milliseconds;return std::nullopt;
    }
    struct DamageResult {
        std::optional<SaveResult> save;
        std::optional<ConcentrationSource> ended;
    };
    // damage_taken is post-defense damage, BEFORE subtracting Temporary HP.
    // Check incapacitation/death after applying damage; these end without a roll.
    DamageResult damage(int damage_taken,int constitution_save,RollModifiers modifiers,
                        bool incapacitated_or_dead,std::uint64_t& rng) {
        if(damage_taken<0)throw std::runtime_error("Invalid concentration damage");
        if(!active_)return {};
        if(incapacitated_or_dead)return {std::nullopt,end()};
        if(!damage_taken)return {};
        auto save=saving_throw(Ability::constitution,constitution_save,
                              std::clamp(damage_taken/2,10,30),modifiers,rng);
        return {save,save.success?std::nullopt:end()};
    }
    std::optional<ConcentrationSource> incapacitate() { return end(); }
    bool operator==(const ConcentrationState&) const = default;
private:
    std::optional<Concentration> active_;
};
}
#endif
