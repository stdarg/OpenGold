#ifndef OPENGOLD_RULES_H
#define OPENGOLD_RULES_H
#include "opengold/message.h"
#include <compare>
#include <array>
#include <cstdint>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace opengold::rules {
struct CharacterSheet;
struct AbilityCheckModifier;
struct FeatureGrant;
struct AdvancementChoice {
    std::string feat;
    std::array<unsigned,6> abilities{};
    std::vector<std::string> spells;
    bool operator==(const AdvancementChoice&) const = default;
};
struct AdvancementOption {std::string id,label,description;bool available{true};};
struct AdvancementOptions {
    unsigned level{};
    std::vector<AdvancementOption> feats,spells;
    std::string description;
};
// Zero selects the equipped weapon's minimum required hands.
struct EquipmentState {
    unsigned weapon_hands{};
    bool operator==(const EquipmentState&) const = default;
};
struct GripOption { unsigned hands{}; Message label; bool available{true}; };
struct CharacterProfile {
    std::string data;
    int hit_points{}, armor_class{};
    std::string description;
    int movement_feet{}, melee_attack_bonus{};
    std::string item_modifiers, spell_modifiers;
    bool strength_dexterity_disadvantage{};
    std::vector<Message> item_messages, spell_messages;
    EquipmentState equipment;
    std::vector<GripOption> grips;
};
enum class EquipmentSlot { unsupported, weapon, armor, shield };
struct EquipmentInfo { EquipmentSlot slot{EquipmentSlot::unsupported}; unsigned hands{}; };
// Module-owned continuation, separate from encounter turn budgets.
struct VitalState {
    int hit_points{};
    bool dead{};
    std::string resources;
    std::string description;
    bool operator==(const VitalState&) const = default;
};
struct TemporaryHitPoints {
    int amount{};
    std::string source_id;
    bool operator==(const TemporaryHitPoints&) const = default;
};
enum class TemporaryHpChoice { keep_current, use_new };
struct RestPolicy {
    unsigned duration_minutes{}, wait_after_rest_minutes{};
};
struct ResourcePool {
    std::string id;
    Message label;
    unsigned remaining{}, capacity{}, short_rest_recovery{};
};
struct RecoveryInfo {
    unsigned hit_die{}, hit_dice{}, hit_dice_max{};
    bool can_rest{};
    std::vector<ResourcePool> resources;
    TemporaryHitPoints temporary_hp;
};
struct HitDieResult {
    unsigned die{};
    int roll{}, modifier{}, healing{};
    unsigned remaining{};
};
using EntityId = std::uint32_t;
struct Cell { int x{}, y{}; auto operator<=>(const Cell&) const = default; };
struct Battlefield {
    int width{}, height{};
    // Geometry facts: 0=open, 1=opaque obstacle, 2=difficult terrain.
    std::vector<std::uint8_t> terrain;
    [[nodiscard]] bool contains(Cell p) const noexcept;
    [[nodiscard]] unsigned at(Cell p) const noexcept;
};
struct Participant {
    EntityId id{};
    std::string definition, name;
    unsigned side{}; // 0=party, 1=opposition in this first encounter adapter.
    Cell cell;
    std::string character_profile;
    std::optional<VitalState> state;
    bool surprised{}; // The rules module determines the mechanical effect.
    bool facing_left{};
};
struct Encounter { Battlefield battlefield; std::vector<Participant> participants; std::uint64_t scope{1}; };
struct Identity {
    std::string module, version, content;
    auto operator<=>(const Identity&) const = default;
};
enum class Outcome { ongoing, victory, defeat };
struct CombatantView {
    EntityId id{};
    std::string name, definition;
    unsigned side{};
    Cell cell;
    int hit_points{}, max_hit_points{}, armor_class{}, initiative{}, movement_feet{};
    bool action{}, bonus_action{}, reaction{}, conscious{}, dead{}, facing_left{};
    std::string status;
    VitalState persistent;
    std::vector<Message> status_messages;
    std::vector<Message> conditions; // Derived display state; mechanics stay in the module.
    std::string type_name, melee_weapon, ranged_weapon; // Rules-owned combat display data.
    bool ranged_attack_available{};
    EquipmentState equipment;
    std::vector<GripOption> grips;
    TemporaryHitPoints temporary_hp;
    std::vector<ResourcePool> resources;
    std::vector<Message> hp_messages;
};
struct TemporaryHpOffer {
    EntityId recipient{};
    TemporaryHitPoints current, offered;
};
struct Snapshot {
    Identity identity;
    std::uint64_t revision{};
    unsigned round{};
    EntityId actor{};
    Outcome outcome{};
    Battlefield battlefield;
    std::vector<CombatantView> combatants; // Initiative order.
    std::vector<std::string> log;
    std::vector<Message> log_messages;
    bool reaction_pending{};
    std::uint64_t elapsed_milliseconds{};
    std::optional<TemporaryHpOffer> temporary_hp_offer;
};
// Verbs are owned by a module, not an enumeration of edition-specific rules.
// Presentation submits only currently offered commands. The module revalidates.
struct Command {
    std::uint64_t revision{};
    EntityId actor{}, target{};
    std::string verb, label;
    Cell destination;
};
class CombatSession {
public:
    virtual ~CombatSession() = default;
    [[nodiscard]] virtual Snapshot snapshot() const = 0;
    [[nodiscard]] virtual std::vector<Command> legal_commands() const = 0;
    // Preview the remaining movement range of a combatant, including one
    // selected outside its turn. Only legal_commands() can authorize a move.
    [[nodiscard]] virtual std::vector<Cell> movement_reach(EntityId actor) const = 0;
    virtual bool submit(const Command& command) = 0;
    [[nodiscard]] virtual std::string save() const = 0;
};
class RulesModule {
public:
    virtual ~RulesModule() = default;
    [[nodiscard]] virtual Identity identity() const = 0;
    [[nodiscard]] virtual bool accepts_campaign_identity(const Identity& saved) const {return saved==identity();}
    [[nodiscard]] virtual std::vector<std::string> supported_features() const = 0;
    [[nodiscard]] virtual std::unique_ptr<CombatSession> create(Encounter encounter, std::uint64_t seed) const = 0;
    [[nodiscard]] virtual std::unique_ptr<CombatSession> restore(std::string_view checkpoint) const = 0;
    [[nodiscard]] virtual CharacterProfile character_profile(const CharacterSheet&, std::span<const std::string>, EquipmentState equipment={}) const;
    [[nodiscard]] virtual EquipmentState migrate_equipment(std::span<const std::string>) const {return {};}
    [[nodiscard]] virtual EquipmentInfo equipment_info(std::string_view) const {return {};}
    [[nodiscard]] virtual AbilityCheckModifier ability_check(const CharacterSheet&,std::span<const std::string> gear,
        unsigned ability,std::string_view skill={},std::string_view tool={},EquipmentState equipment={}) const;
    [[nodiscard]] virtual unsigned experience_for_level(unsigned level) const;
    // False means this module's supported advancement ceiling was reached.
    virtual bool advance_character(CharacterSheet& sheet, VitalState& state) const;
    [[nodiscard]] virtual AdvancementOptions advancement_options(const CharacterSheet&) const {return {};}
    [[nodiscard]] virtual AdvancementChoice default_advancement(const CharacterSheet&) const {return {};}
    virtual bool advance_character(CharacterSheet& sheet,VitalState& state,const AdvancementChoice&) const;
    virtual void recover(VitalState& state, const CharacterSheet& sheet) const;
    [[nodiscard]] virtual RecoveryInfo recovery_info(const CharacterSheet&,const VitalState&) const;
    // A granting feature must establish entitlement and spend its costs before
    // calling this operation. The choice is explicit; pools never stack.
    virtual void grant_temporary_hit_points(VitalState&,const CharacterSheet&,const TemporaryHitPoints&,TemporaryHpChoice) const;
    // The campaign must establish completed-rest eligibility before invoking
    // these resource operations. Each spend commits one die and its RNG draw.
    virtual void recover_short_rest(VitalState&,const CharacterSheet&) const;
    virtual HitDieResult spend_hit_die(VitalState&,const CharacterSheet&,std::uint64_t&) const;
    // Advances module-owned lasting effects for a group in deterministic order.
    virtual void elapse(std::span<Participant>, std::uint64_t, std::uint64_t&) const {}
    virtual void validate_character_state(const CharacterSheet&, const VitalState&) const;
    virtual void validate_saved_grants(const Identity&,const CharacterSheet&,std::span<const FeatureGrant>) const;
    // Called after replaying saved creation/advancement under the current rules.
    // Edition-specific migration preserves wounds and opaque resource state.
    virtual void migrate_character_state(const Identity&, const CharacterSheet& sheet, VitalState& state) const
    {validate_character_state(sheet,state);}
    [[nodiscard]] virtual RestPolicy long_rest_policy() const;
    [[nodiscard]] virtual RestPolicy short_rest_policy() const;
    virtual void set_hit_points(VitalState&,const CharacterSheet&,int) const;
    virtual void temple_heal(VitalState& state, const CharacterSheet& sheet, std::uint64_t& random_state) const;
};
}
#endif
