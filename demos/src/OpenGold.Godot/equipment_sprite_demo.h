#ifndef OPENGOLD_EQUIPMENT_SPRITE_DEMO_H
#define OPENGOLD_EQUIPMENT_SPRITE_DEMO_H
#include "opengold/campaign_party.h"
#include "opengold/combat_body_catalog.h"
#include <godot_cpp/classes/control.hpp>
#include <optional>

// Isolated artwork fixture: no campaign, saves, or catalog writes.
class EquipmentSpriteDemo : public godot::Control {
    GDCLASS(EquipmentSpriteDemo, godot::Control)
public:
    void _ready() override;
    void _input(const godot::Ref<godot::InputEvent>& event) override;
protected:
    static void _bind_methods() {}
    void _notification(int what);
private:
    std::optional<opengold::PartyMember> member_;
    std::optional<opengold::por::CharacterArt> art_;
    opengold::por::CombatBodyCatalog catalog_;
    std::vector<unsigned> hands_;
    int selected_{};
    bool ready_{};
    void layout();
    void refresh();
    void select(std::int64_t index);
    void change_equipment(bool equip);
};
#endif
