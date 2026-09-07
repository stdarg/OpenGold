#include "opengold/character.h"

namespace opengold {
Character::Character(const rules::CharacterRules& rules,rules::CharacterDraft creation,por::CharacterAppearance appearance)
    :creation_(std::move(creation)),sheet_(rules.evaluate(creation_,true))
{this->appearance(appearance);}
void Character::appearance(por::CharacterAppearance value)
{por::validate_character_appearance(value);appearance_=value;}
}
