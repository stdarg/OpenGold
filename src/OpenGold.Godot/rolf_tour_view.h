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
protected:
    static void _bind_methods();
    void _notification(int what);
private:
    std::optional<opengold::por::RolfTourSession> session_;
    std::array<godot::Ref<godot::ImageTexture>, 3> sprites_;
    godot::Ref<godot::AudioStreamWAV> footstep_;
    godot::Rect2 scene_rect_, map_rect_, dialogue_rect_;
    godot::String error_;
    std::uint64_t shown_revision_{}, checked_ticket_{};
    unsigned played_footsteps_{}, check_frames_{}, check_prompts_{};
    bool full_map_{true}, ready_{}, checking_{}, capture_{}, capture_pending_{};
    void layout();
    void refresh();
    void restart();
    void next();
    void left();
    void right();
    void forward();
    void map_mode();
    void movement(opengold::por::ExplorationCommand command);
    void draw_scene();
    void draw_map();
    void check_run();
    void capture_frame(const godot::String& name);
};
#endif
