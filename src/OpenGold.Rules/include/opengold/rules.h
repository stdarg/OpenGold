#ifndef OPENGOLD_RULES_H
#define OPENGOLD_RULES_H
#include <compare>
#include <cstdint>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace opengold::rules {
struct CharacterSheet;
struct CharacterProfile {
    std::string data;
    int hit_points{}, armor_class{};
    std::string description;
    int movement_feet{}, melee_attack_bonus{};
};
// Module-owned continuation, separate from encounter turn budgets.
struct VitalState {
    int hit_points{};
    bool dead{};
    std::string resources;
    std::string description;
    bool operator==(const VitalState&) const = default;
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
    [[nodiscard]] virtual std::vector<std::string> supported_features() const = 0;
    [[nodiscard]] virtual std::unique_ptr<CombatSession> create(Encounter encounter, std::uint64_t seed) const = 0;
    [[nodiscard]] virtual std::unique_ptr<CombatSession> restore(std::string_view checkpoint) const = 0;
    [[nodiscard]] virtual CharacterProfile character_profile(const CharacterSheet&, std::span<const std::string>) const;
};
}
#endif
