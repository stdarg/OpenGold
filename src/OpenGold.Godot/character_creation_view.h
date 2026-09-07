#ifndef OPENGOLD_CHARACTER_CREATION_VIEW_H
#define OPENGOLD_CHARACTER_CREATION_VIEW_H
#include "opengold/character_creator.h"
#include "opengold/campaign_party.h"
#include <godot_cpp/classes/control.hpp>
#include <godot_cpp/classes/image_texture.hpp>
#include <optional>
#include <functional>

class CharacterCreationView : public godot::Control {
    GDCLASS(CharacterCreationView,godot::Control)
public:
    void _ready() override;
    void _draw() override;
    void _process(double delta) override;
protected:
    static void _bind_methods();
    void _notification(int what);
private:
    std::unique_ptr<opengold::CharacterCreator> creator_;
    std::optional<opengold::Character> completed_;
    std::optional<opengold::por::CharacterArt> art_;
    std::optional<opengold::por::CharacterAppearance> rendered_;
    std::array<godot::Ref<godot::ImageTexture>,3> images_;
    godot::Rect2 page_rect_,preview_rect_,portrait_rect_,ready_rect_,action_rect_;
    godot::String error_;
    int selected_score_{-1},color_bank_{},color_part_{};
    unsigned check_stage_{},check_frames_{},check_head_{},check_default_{};
    unsigned drag_check_stage_{},modal_check_stage_{};
    bool ready_{},refreshing_{},checking_{},capture_{},fatal_{},portrait_chosen_{};
    void layout();
    void refresh();
    void refresh_art();
    void load_additional_heads();
    void recommend_head();
    void perform(const std::function<void()>& action);
    void next();
    void back();
    void restart();
    void choice_selected(std::int64_t index);
    void background_selected(std::int64_t index);
    void bonus_selected(std::int64_t index);
    void roll();
    void score_selected(int index);
    godot::Variant drag_roll(godot::Vector2 position,int index);
    bool can_drop_roll(godot::Vector2 position,const godot::Variant& data,int index);
    void drop_roll(godot::Vector2 position,const godot::Variant& data,int index);
    godot::String sheet_text(const opengold::Character& character,const opengold::PartyMember* member=nullptr) const;
    void show_modifiers();
    void close_modifiers();
    void name_changed(godot::String value);
    void portrait_part(int part,int direction);
    void portrait_head_selected(std::int64_t index);
    void combat_part(int part,int direction);
    void toggle_size();
    void color_selected(int bank,int part);
    void palette_selected(int index);
    void check_run();
    void capture(const char* name);
    void capture_portrait_armor();
    std::shared_ptr<opengold::CampaignParty> campaign_;
    std::size_t roster_index_{};
    bool party_open_{},added_to_party_{},party_check_{};
    unsigned party_check_stage_{};
    void setup_party();
    void party_action(int action);
    void party_selected(std::int64_t index);
    void refresh_party();
    void party_layout();
    void party_check();
    void update_party_navigation();
};
#endif
