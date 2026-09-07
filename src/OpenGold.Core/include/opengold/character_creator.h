#ifndef OPENGOLD_CHARACTER_CREATOR_H
#define OPENGOLD_CHARACTER_CREATOR_H
#include "opengold/character.h"
namespace opengold {
enum class CreationStep { race, gender, character_class, alignment, attributes, name, portrait, combat_icon, sheet };
class CharacterCreator {
public:
    CharacterCreator(std::unique_ptr<rules::CharacterRules> rules,std::uint64_t seed);
    [[nodiscard]] const rules::CharacterRules& rules() const {return *rules_;}
    [[nodiscard]] const rules::CharacterDraft& draft() const {return draft_;}
    [[nodiscard]] const por::CharacterAppearance& appearance() const {return appearance_;}
    [[nodiscard]] CreationStep step() const {return step_;}
    [[nodiscard]] rules::CharacterSheet sheet() const;
    [[nodiscard]] Character create_character() const;
    void select(rules::CreationField field,std::string_view id);
    void select_adjustment(unsigned index);
    void roll();
    void assign_roll(unsigned roll,unsigned ability);
    [[nodiscard]] bool scores_assigned() const;
    void swap_scores(unsigned first,unsigned second);
    void name(std::string text);
    void appearance(por::CharacterAppearance value);
    void next();
    void back();
    void restart();
private:
    std::unique_ptr<rules::CharacterRules> rules_;
    std::uint64_t random_;
    rules::CharacterDraft draft_;
    por::CharacterAppearance appearance_;
    CreationStep step_{CreationStep::race};
    void require_editable() const;
};
}
#endif
