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
    require(not combat.has_node("Title") and not combat.has_node("Subtitle"),
        "Demo uses the header-free campaign combat layout")
    require(combat.get_node("BattlefieldScroll").position.y == 16,
        "Demo battlefield reaches the top of the combat screen")
    require(combat.get_node("ZoomLevel").text == "200%", "Demo shows the combat zoom level")
    require(combat.get_node("Footer").text == "Each square is 5 feet. Victory returns your party to exploration.",
        "Demo uses the campaign combat footer")
    require(combat.get_node("Roster").text.contains("Kobold"), "Kobolds appear in the combat roster")
    require(combat.get_node("Log").text.contains("The original script has requested combat."),
        "Demo uses the campaign encounter log presentation")
    require(combat.get_node("BattlefieldScroll/Canvas").size.x > combat.get_node("BattlefieldScroll").size.x,
        "Showcase battlefield uses the game's configured zoom")
    await RenderingServer.frame_post_draw
    var output := ProjectSettings.globalize_path("res://../../../build/checks/combat-demo.png")
    require(root.get_texture().get_image().save_png(output) == OK, "Showcase screenshot saves")
    print("Combat demo checks passed: ", output)
    quit(0)
