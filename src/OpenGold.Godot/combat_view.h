#ifndef OPENGOLD_COMBAT_VIEW_H
#define OPENGOLD_COMBAT_VIEW_H
#include "opengold/combat_demo.h"
#include <godot_cpp/classes/control.hpp>
#include <godot_cpp/classes/input_event.hpp>
#include <godot_cpp/classes/image_texture.hpp>
#include <map>
class CombatView : public godot::Control {
    GDCLASS(CombatView,godot::Control)
public:
    void _ready() override;
    void _process(double delta) override;
    void _draw() override;
    void _input(const godot::Ref<godot::InputEvent>& event) override;
    void campaign_party(std::shared_ptr<opengold::CampaignParty> party,std::vector<opengold::CombatArt> art) {campaign_=std::move(party);campaign_art_=std::move(art);}
    [[nodiscard]] bool can_leave() const {return !demo_||!demo_->has_combat()||demo_->combat().snapshot().outcome!=opengold::rules::Outcome::ongoing;}
protected:
    static void _bind_methods();
    void _notification(int what);
private:
    std::unique_ptr<opengold::CombatDemo> demo_;
    std::shared_ptr<opengold::CampaignParty> campaign_;
    std::vector<opengold::CombatArt> campaign_art_;
    std::map<opengold::rules::EntityId,godot::Ref<godot::ImageTexture>> art_;
    godot::Rect2 board_rect_;
    std::string mode_{"move"},error_;
    double ai_delay_{};
    bool ready_{},checking_{},capture_{},captured_{},check_slums_{},checked_input_{};
    bool party_check_{};
    unsigned check_steps_{},completion_frames_{};
    void layout();void refresh();void sync_art();void act(const opengold::rules::Command& command);
    void select_mode(godot::String verb);void immediate(godot::String verb);
    void training();void slums();void replay();void next();void revisit();void save_game();void load_game();
    std::filesystem::path local_path(const char* path) const;
};
#endif
