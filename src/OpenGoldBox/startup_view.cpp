#include "startup_view.h"
#include <godot_cpp/classes/os.hpp>
#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/classes/input_event_key.hpp>
#include <godot_cpp/classes/texture_rect.hpp>
#include <godot_cpp/classes/texture2d.hpp>
#include <godot_cpp/classes/viewport.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

void StartupView::_bind_methods()
{
    ClassDB::bind_method(D_METHOD("open_character_creation"), &StartupView::open_character_creation);
}

void StartupView::_ready()
{
    auto* os = OS::get_singleton(); // borrowed engine singleton
    if (!os->get_cmdline_args().has("--splash") && !os->get_cmdline_user_args().has("--splash")) {
        finish();
        return;
    }
    show_screen();
}

void StartupView::show_screen()
{
    const char* path = screen_ == 0 ? "res://bin/splashes/OpenGoldBoxSplash.png"
                                  : "res://bin/splashes/OpenGoldBoxPoolOfRadiance.png";
    Ref<Texture2D> texture = ResourceLoader::get_singleton()->load(path);
    if (texture.is_null()) {
        UtilityFunctions::push_error(String("Missing splash image: ") + path);
        finish();
        return;
    }
    get_node<TextureRect>("Image")->set_texture(texture);
}

void StartupView::_input(const Ref<InputEvent>& event)
{
    const Ref<InputEventKey> key = event;
    if (finishing_ || key.is_null() || !key->is_pressed() || key->is_echo()) return;
    // The application-wide shutdown shortcut must not advance a splash.
    if (key->is_ctrl_pressed() && key->get_keycode() == KEY_X) return;
    get_viewport()->set_input_as_handled();
    if (key->get_keycode() == KEY_ESCAPE || screen_ == 1) {
        finish();
    } else {
        ++screen_;
        show_screen();
    }
}

void StartupView::finish()
{
    if (finishing_) return;
    finishing_ = true;
    // Scene changes must occur after ready/input dispatch has completed.
    call_deferred("open_character_creation");
}

void StartupView::open_character_creation()
{
    if (get_tree()->change_scene_to_file("res://scenes/character_creation.tscn") != OK) {
        UtilityFunctions::push_error("Cannot open character creation.");
        get_tree()->quit(1);
    }
}
