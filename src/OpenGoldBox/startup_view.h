#ifndef OPENGOLDBOX_STARTUP_VIEW_H
#define OPENGOLDBOX_STARTUP_VIEW_H

#include <godot_cpp/classes/control.hpp>
#include <godot_cpp/classes/input_event.hpp>

class StartupView : public godot::Control {
    GDCLASS(StartupView, godot::Control)
public:
    void _ready() override;
    void _notification(int what);
    void _input(const godot::Ref<godot::InputEvent>& event) override;
protected:
    static void _bind_methods();
private:
    unsigned screen_{};
    bool finishing_{};
    void show_screen();
    void layout_text();
    void finish();
    void open_character_creation();
};

#endif
