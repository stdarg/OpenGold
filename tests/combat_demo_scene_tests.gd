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
    require(combat.get_node("ZoomLevel").text == "100%", "Demo shows the combat zoom level")
    require(combat.get_node("Footer").text.contains("A: next action"),
        "Demo explains the combat keyboard controls")
    require(not combat.get_node("Roster").visible and not combat.get_node("Turn").visible,
        "The upper-right text window is removed")
    require(combat.get_node("Log").text.contains("Dorian Nightwind turn"),
        "Turn text moved below the battlefield")
    require(combat.get_node("Log").text.contains("The original script has requested combat."),
        "Demo uses the campaign encounter log presentation")
    require(combat.get_node("ZoomLevel").text == "100%", "Showcase uses 100 percent zoom")
    await RenderingServer.frame_post_draw
    var screenshot := root.get_texture().get_image()
    require(screenshot.get_pixel(1015, 415).r > screenshot.get_pixel(1140, 415).r,
        "Reachable movement squares visibly lighten in the surrounded demo")
    var click := InputEventMouseButton.new()
    click.button_index = MOUSE_BUTTON_LEFT
    click.pressed = true
    click.position = Vector2(1550, 90)
    root.push_input(click)
    await RenderingServer.frame_post_draw
    var other_selected := root.get_texture().get_image()
    require(other_selected.get_pixel(1015, 415).r < screenshot.get_pixel(1015, 415).r,
        "Selecting a different party portrait changes the movement display")
    click.position = Vector2(1550, 330)
    root.push_input(click)
    await RenderingServer.frame_post_draw
    screenshot = root.get_texture().get_image()
    require(screenshot.get_pixel(1015, 415).r > other_selected.get_pixel(1015, 415).r,
        "Selecting the active hero restores their legal movement squares")
    var output := ProjectSettings.globalize_path("res://../../../build/checks/combat-demo.png")
    require(screenshot.save_png(output) == OK, "Showcase screenshot saves")
    print("Combat demo checks passed: ", output)
    quit(0)
