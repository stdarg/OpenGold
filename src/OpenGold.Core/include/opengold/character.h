#ifndef OPENGOLD_CHARACTER_H
#define OPENGOLD_CHARACTER_H
#include "opengold/character_art.h"
#include "opengold/character_rules.h"
#include "opengold/inventory.h"

namespace opengold {
// A finished character owns its data. It does not borrow from the creator,
// rules module, art archives or Godot, and can be copied into a roster later.
class Character {
public:
    Character(const rules::CharacterRules& rules,rules::CharacterDraft creation,por::CharacterAppearance appearance);
    [[nodiscard]] const rules::CharacterDraft& creation_data() const {return creation_;}
    [[nodiscard]] const rules::CharacterSheet& sheet() const {return sheet_;}
    [[nodiscard]] const por::CharacterAppearance& appearance() const {return appearance_;}
    void appearance(por::CharacterAppearance value);
    [[nodiscard]] const Inventory& inventory() const {return inventory_;}
    [[nodiscard]] Inventory& inventory() {return inventory_;}
private:
    rules::CharacterDraft creation_;
    rules::CharacterSheet sheet_;
    por::CharacterAppearance appearance_;
    Inventory inventory_;
};
}
#endif
