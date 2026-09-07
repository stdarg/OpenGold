#ifndef OPENGOLD_CHARACTER_RULES_H
#define OPENGOLD_CHARACTER_RULES_H
#include "opengold/rules.h"
#include <array>

namespace opengold::rules {
enum class CreationField { race, gender, character_class, alignment, background };
struct CreationChoice { std::string id, label, description; };
struct AbilityRoll {
    std::array<int,4> dice{};
    unsigned discarded{};
    [[nodiscard]] int total() const;
    bool operator==(const AbilityRoll&) const = default;
};
struct ScoreAdjustment {
    std::string label;
    std::array<int,6> bonuses{};
};
struct CharacterDraft {
    std::string race, gender, character_class, alignment, background, name;
    std::array<AbilityRoll,6> rolls{};
    // Each assigned ability owns a unique result; 6 means not yet assigned.
    std::array<unsigned,6> assignment{0,1,2,3,4,5};
    unsigned adjustment{};
    bool rolled{};
};
struct CharacterSheet {
    Identity identity;
    std::string name, race, gender, character_class, alignment, background;
    std::array<int,6> base{}, bonuses{}, scores{}, modifiers{};
    int level{1}, hit_die{}, hit_points{};
    std::string hp_explanation;
    std::array<int,6> saving_throws{};
    std::array<bool,6> save_proficiencies{};
    std::string racial_modifiers, class_modifiers, background_modifiers;
};
// Creation is a separate optional capability: campaign and Godot code do not
// embed edition-specific tables, rolling policies, or HP arithmetic.
class CharacterRules {
public:
    virtual ~CharacterRules() = default;
    [[nodiscard]] virtual Identity identity() const = 0;
    [[nodiscard]] virtual std::vector<CreationChoice> choices(CreationField field) const = 0;
    [[nodiscard]] virtual std::vector<ScoreAdjustment> adjustments(std::string_view background) const = 0;
    [[nodiscard]] virtual std::array<AbilityRoll,6> roll(std::uint64_t& random_state) const = 0;
    [[nodiscard]] virtual CharacterSheet evaluate(const CharacterDraft& draft, bool require_name) const = 0;
};
}
#endif
