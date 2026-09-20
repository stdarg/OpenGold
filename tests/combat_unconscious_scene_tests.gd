extends SceneTree

func _initialize() -> void:
    call_deferred("check")

func check() -> void:
    root.size = Vector2i(1920, 1080)
    change_scene_to_file("res://scenes/combat_demo.tscn")
    for frame in range(1500):
        await process_frame
        var combat := current_scene
        if combat == null:
            continue
        if combat.get_node("Log").text.contains("Oren Quickwater falls unconscious."):
            await RenderingServer.frame_post_draw
            var path := ProjectSettings.globalize_path("res://../../../build/checks/combat-unconscious.png")
            if root.get_texture().get_image().save_png(path) != OK:
                push_error("Could not capture the unconscious combatant")
                quit(1)
                return
            if combat.get_node("DeathAudio").playing:
                push_error("Unconsciousness incorrectly played the death sound")
                quit(1)
                return
            print("Unconscious combatant captured: ", path)
            quit(0)
            return
        var end_key := InputEventKey.new()
        end_key.keycode = KEY_ENTER
        end_key.pressed = true
        root.push_input(end_key)
    push_error("Combat did not reach an unconscious party member")
    quit(1)
