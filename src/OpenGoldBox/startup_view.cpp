#include "startup_view.h"
#include <godot_cpp/classes/os.hpp>
#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/classes/input_event_key.hpp>
#include <godot_cpp/classes/texture_rect.hpp>
#include <algorithm>
#include <godot_cpp/classes/texture2d.hpp>
#include <godot_cpp/classes/viewport.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;
namespace { constexpr double text_fade_seconds = 0.6; }

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
    auto* image = get_node<TextureRect>("Image"); // scene-owned
    // Load once: advancing changes only text, never the backdrop texture or geometry.
    if (image->get_texture().is_null()) {
        Ref<Texture2D> texture = ResourceLoader::get_singleton()->load("res://bin/splashes/OpenGoldBoxSplashBackground.png");
        if (texture.is_null()) {
            UtilityFunctions::push_error("Missing shared splash background");
            finish();
            return;
        }
        image->set_texture(texture);
    }
    const char* lettering_path = screen_ == 0
        ? "res://bin/splashes/OpenGoldBoxEngineLettering.png"
        : "res://bin/splashes/OpenGoldBoxGameLettering.png";
    Ref<Texture2D> lettering = ResourceLoader::get_singleton()->load(lettering_path);
    if (lettering.is_null()) {
        UtilityFunctions::push_error(String("Missing splash lettering: ") + lettering_path);
        finish();
        return;
    }
    auto* text = get_node<TextureRect>("Text"); // scene-owned
    text->set_texture(lettering);
    text->set_self_modulate(Color(1, 1, 1, 0));
    fade_elapsed_ = 0;
    set_process(true);
    layout_text();
}

void StartupView::_process(double delta)
{
    if (finishing_) return;
    fade_elapsed_ = std::min(text_fade_seconds, fade_elapsed_ + std::max(0.0, delta));
    get_node<TextureRect>("Text")->set_self_modulate(Color(1, 1, 1, fade_elapsed_ / text_fade_seconds));
    if (fade_elapsed_ >= text_fade_seconds) set_process(false);
}

void StartupView::_notification(int what)
{
    if (what == NOTIFICATION_RESIZED && is_node_ready()) layout_text();
}

void StartupView::layout_text()
{
    // The text uses the same centered, uniform fit as the shared background.
    const auto texture = get_node<TextureRect>("Image")->get_texture();
    if (texture.is_null()) return;
    const auto source = texture->get_size();
    const double fit = std::min(get_size().x / source.x, get_size().y / source.y);
    const auto fitted = source * fit;
    const auto origin = (get_size() - fitted) * .5;
    // Keep the lettering proportions and comfortable margins from the reference.
    const auto lettering_size = fitted * .8;
    auto* lettering = get_node<TextureRect>("Text"); // scene-owned
    lettering->set_position(origin + (fitted - lettering_size) * .5 - Vector2(0, fitted.y * .03));
    lettering->set_size(lettering_size);
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
    set_process(false);
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
