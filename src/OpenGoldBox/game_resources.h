#ifndef OPENGOLDBOX_GAME_RESOURCES_H
#define OPENGOLDBOX_GAME_RESOURCES_H

#include <godot_cpp/classes/os.hpp>
#include <godot_cpp/classes/project_settings.hpp>

// The native rules loader reads ordinary files, not Godot's packed resources.
inline godot::String game_rules_file()
{
    constexpr auto relative = "data/rules/srd-5.2.1/combat.rules";
    auto* os = godot::OS::get_singleton(); // borrowed engine singleton
    if (os->has_feature("editor")) {
        return godot::ProjectSettings::get_singleton()->globalize_path(
            godot::String("res://") + relative);
    }
    return os->get_executable_path().get_base_dir().path_join(relative);
}

#endif
