extends SceneTree

func _initialize() -> void:
    call_deferred("check_demo")

func require(ok: bool, message: String) -> void:
    if not ok:
        push_error(message)
        quit(1)
        assert(ok, message)

func check_demo() -> void:
    root.size = Vector2i(1920, 1080)
    change_scene_to_file("res://scenes/combat_demo.tscn")
    for frame in range(8):
        await process_frame
    var combat := current_scene
    require(combat.get_node("Title").text.contains("Kobold encirclement"), "Showcase title loads")
    require(combat.get_node("Roster").text.contains("Kobold"), "Kobolds appear in the combat roster")
    require(combat.get_node("BattlefieldScroll/Canvas").size.x > combat.get_node("BattlefieldScroll").size.x,
        "Showcase battlefield uses the game's configured zoom")
    await RenderingServer.frame_post_draw
    var output := ProjectSettings.globalize_path("res://../../../build/checks/kobold-demo.png")
    require(root.get_texture().get_image().save_png(output) == OK, "Showcase screenshot saves")
    print("Kobold combat demo checks passed: ", output)
    quit(0)
