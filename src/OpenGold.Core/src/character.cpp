#include "opengold/character.h"

namespace opengold {
Character::Character(const rules::CharacterRules& rules,rules::CharacterDraft creation,por::CharacterAppearance appearance)
    :creation_(std::move(creation)),sheet_(rules.evaluate(creation_,true))
{this->appearance(appearance);}
void Character::appearance(por::CharacterAppearance value)
{por::validate_character_appearance(value);appearance_=value;}
bool Character::advance(const rules::RulesModule& rules, rules::VitalState& state)
{return advance(rules,state,rules.default_advancement(sheet_));}
bool Character::advance(const rules::RulesModule& rules,rules::VitalState& state,const rules::AdvancementChoice& choice)
{
    auto sheet=sheet_;auto vitals=state;auto history=advancements_;
    if(!rules.advance_character(sheet,vitals,choice))return false;
    history.push_back(choice);sheet_=std::move(sheet);state=std::move(vitals);advancements_=std::move(history);return true;
}
}
