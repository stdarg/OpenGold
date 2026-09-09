#ifndef OPENGOLD_RULES_H
#define OPENGOLD_RULES_H
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
struct CharacterProfile {
    std::string data;
    int hit_points{}, armor_class{};
    std::string description;
    int movement_feet{}, melee_attack_bonus{};
    std::string item_modifiers, spell_modifiers;
    bool strength_dexterity_disadvantage{};
};
// Module-owned continuation, separate from encounter turn budgets.
struct VitalState {
    int hit_points{};
    bool dead{};
    std::string resources;
    std::string description;
    bool operator==(const VitalState&) const = default;
};
struct RestPolicy {
    unsigned duration_minutes{}, wait_after_rest_minutes{};
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
};
struct Encounter { Battlefield battlefield; std::vector<Participant> participants; };
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
    bool action{}, bonus_action{}, reaction{}, conscious{}, dead{};
    std::string status;
    VitalState persistent;
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
    bool reaction_pending{};
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
    [[nodiscard]] virtual CharacterProfile character_profile(const CharacterSheet&, std::span<const std::string>) const;
    [[nodiscard]] virtual unsigned experience_for_level(unsigned level) const;
    // False means this module's supported advancement ceiling was reached.
    virtual bool advance_character(CharacterSheet& sheet, VitalState& state) const;
    [[nodiscard]] virtual AdvancementOptions advancement_options(const CharacterSheet&) const {return {};}
    [[nodiscard]] virtual AdvancementChoice default_advancement(const CharacterSheet&) const {return {};}
    virtual bool advance_character(CharacterSheet& sheet,VitalState& state,const AdvancementChoice&) const;
    virtual void recover(VitalState& state, const CharacterSheet& sheet) const;
    virtual void validate_character_state(const CharacterSheet&, const VitalState&) const;
    [[nodiscard]] virtual RestPolicy long_rest_policy() const;
    virtual void temple_heal(VitalState& state, const CharacterSheet& sheet, std::uint64_t& random_state) const;
};
}
#endif
