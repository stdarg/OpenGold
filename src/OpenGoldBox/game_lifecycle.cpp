#include "game_lifecycle.h"
#include <godot_cpp/classes/input_event_key.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/classes/window.hpp>
#include <godot_cpp/variant/callable_method_pointer.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

void GameLifecycle::_bind_methods() {}

void GameLifecycle::_ready()
{
    set_process_mode(PROCESS_MODE_ALWAYS);
    auto* tree = get_tree(); // borrowed; SceneTree owns this autoload
    tree->set_auto_accept_quit(false);
    tree->get_root()->connect("close_requested", callable_mp(this, &GameLifecycle::request_quit));
    watch_window(tree->get_root());
    tree->connect("node_added", callable_mp(this, &GameLifecycle::watch_window));
}

void GameLifecycle::watch_window(Node* node)
{
    // Windows remain scene-owned. Signal connections are removed with their objects.
    if (auto* window = Object::cast_to<Window>(node)) {
        window->connect("window_input", callable_mp(this, &GameLifecycle::_input));
    }
}

void GameLifecycle::_input(const Ref<InputEvent>& event)
{
    const Ref<InputEventKey> key = event;
    if (key.is_null() || !key->is_pressed() || key->is_echo() ||
        !key->is_ctrl_pressed() || key->get_keycode() != KEY_X) return;
    get_viewport()->set_input_as_handled();
    request_quit();
}

void GameLifecycle::request_quit()
{
    if (quitting_) return;
    quitting_ = true;
    UtilityFunctions::print("OpenGoldBox: graceful shutdown requested");
    // Finish this iteration, then let Godot free the tree and native RAII owners.
    get_tree()->quit(0);
}
