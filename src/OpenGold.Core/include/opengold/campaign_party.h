#ifndef OPENGOLD_CAMPAIGN_PARTY_H
#define OPENGOLD_CAMPAIGN_PARTY_H
#include "opengold/character.h"
#include "opengold/creature_catalog.h"
#include "opengold/ecl_machine.h"

namespace opengold {
using MemberId = rules::EntityId;
struct PartyMember {
    MemberId id{};
    Character character;
    std::string npc_source; // Empty for PCs; explicit campaign conversion identity for NPCs.
    rules::VitalState vitals;
    std::array<std::uint16_t,7> wealth{};
    std::vector<std::uint64_t> equipped;
    unsigned morale{100};
    unsigned experience{};
    std::optional<std::uint64_t> last_rest_minutes;
    std::map<std::uint64_t,por::Equipment> item_sources;
    std::string creation_source; // Stable pool candidate identity, empty for authored PCs.
};
struct PartyState {
    std::vector<PartyMember> roster;
    std::array<MemberId,8> slots{};
    MemberId next_id{1};
    unsigned selected{};
    std::uint64_t time_minutes{}, random_state{42};
    std::vector<std::string> claimed_rewards;
};
// One shared campaign value store. Sessions share this owner, never separate PCs.
// While combat owns mutable vitals, roster/equipment/script mutations are barred.
class CampaignParty {
public:
    explicit CampaignParty(std::unique_ptr<rules::RulesModule> rules);
    [[nodiscard]] const PartyState& state() const { return state_; }
    [[nodiscard]] const PartyMember& member(MemberId id) const;
    [[nodiscard]] MemberId selected() const {return state_.slots.at(state_.selected);}
    void select(unsigned slot);
    MemberId add_pc(Character character);
    MemberId recruit(std::string source,Character converted,unsigned morale=100);
    void rejoin(MemberId id);
    void remove(MemberId id);
    void equip(MemberId id,std::uint64_t item);
    void unequip(MemberId id,std::uint64_t item);
    void purchase(MemberId id,const por::Equipment& item);
    void set_wealth(MemberId id,std::array<std::uint16_t,7> wealth);
    void award_experience(unsigned amount,std::string reward_id);
    // Atomic original loot delivery. A full set of purses leaves it unclaimed.
    bool award_loot(const std::array<unsigned,7>& wealth,const std::vector<por::Equipment>& items,std::string reward_id);
    [[nodiscard]] bool rest();
    void temple_heal(MemberId target);
    void advance_time(unsigned minutes);
    [[nodiscard]] std::uint64_t time_hours() const noexcept {return state_.time_minutes/60;}
    [[nodiscard]] rules::CharacterProfile profile(MemberId id) const;
    [[nodiscard]] bool has_item(unsigned original_type) const;
    [[nodiscard]] unsigned strength() const;
    [[nodiscard]] std::array<unsigned,4> query(unsigned address,unsigned effect) const;
    [[nodiscard]] por::EclHostReply character_reply(unsigned slot) const;
    void read_character(unsigned slot,const por::EclMachine& vm);
    [[nodiscard]] PartyState checkpoint() const {return state_;}
    void restore(PartyState state);
    static void validate(const PartyState& state);
    [[nodiscard]] std::vector<rules::Participant> participants() const;
    void begin_combat();
    void apply_combat(const rules::Snapshot& snapshot);
    void end_combat() noexcept {combat_=false;}
    [[nodiscard]] bool in_combat() const {return combat_;}
    [[nodiscard]] const rules::Identity identity() const {return rules_->identity();}
private:
    std::unique_ptr<rules::RulesModule> rules_;
    PartyState state_;
    bool combat_{};
    void editable() const;
    PartyMember& edit(MemberId id);
    void join(MemberId id,bool npc);
};
[[nodiscard]] std::string equipment_conversion(const por::Equipment& item);
}
#endif
