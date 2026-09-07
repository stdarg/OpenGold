#ifndef OPENGOLD_ROLF_TOUR_VIEW_H
#define OPENGOLD_ROLF_TOUR_VIEW_H

#include "opengold/rolf_tour.h"
#include <godot_cpp/classes/control.hpp>
#include <godot_cpp/classes/input_event.hpp>
#include <godot_cpp/classes/image_texture.hpp>
#include <godot_cpp/classes/audio_stream_wav.hpp>
#include <optional>

class RolfTourView : public godot::Control {
    GDCLASS(RolfTourView, godot::Control)
public:
    void _ready() override;
    void _process(double delta) override;
    void _draw() override;
    void _input(const godot::Ref<godot::InputEvent>& event) override;
    void campaign_party(std::shared_ptr<opengold::CampaignParty> party){campaign_=std::move(party);embedded_party_=true;}
    [[nodiscard]] bool can_leave() const {return !session_||session_->can_leave();}
    void resume_party(){shown_revision_=0;refresh();}
    [[nodiscard]] bool party_route_checked() const {return shop_check_stage_==4;}
protected:
    static void _bind_methods();
    void _notification(int what);
private:
    std::optional<opengold::por::RolfTourSession> session_;
    std::shared_ptr<opengold::CampaignParty> campaign_;
    std::array<godot::Ref<godot::ImageTexture>, 3> sprites_;
    godot::Ref<godot::ImageTexture> wall_view_;
    std::optional<opengold::por::PartyPose> rendered_pose_;
    godot::Ref<godot::AudioStreamWAV> footstep_;
    godot::Rect2 scene_rect_, map_rect_, dialogue_rect_;
    godot::String error_;
    std::uint64_t shown_revision_{}, checked_ticket_{};
    unsigned played_footsteps_{}, check_frames_{}, check_prompts_{};
    unsigned rendered_sprite_id_{999};
    std::uint64_t displayed_ticket_{};
    std::uint64_t rendered_picture_revision_{};
    bool full_map_{true}, ready_{}, checking_{}, capture_{}, capture_pending_{};
    bool town_check_{},embedded_party_{};
    unsigned shop_check_stage_{};
    void layout();
    void refresh();
    void restart();
    void next();
    void left();
    void right();
    void forward();
    void look();
    void inventory();
    void refresh_inventory();
    void inventory_selected(std::int64_t index);
    void equip_item(bool equip);
    void party_selected(std::int64_t index);
    void close_sheet();
    void leave_shop();
    void map_mode();
    void movement(opengold::por::ExplorationCommand command);
    void draw_scene();
    void draw_map();
    void check_run();
    void check_town();
    void capture_frame(const godot::String& name);
};
#endif
