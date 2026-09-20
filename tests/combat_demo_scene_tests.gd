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
    require(is_equal_approx(combat.get_node("BattlefieldScroll").size.y, (combat.size.y - 180) * 0.85),
        "Demo uses the shorter shared battlefield layout")
    require(combat.get_node("ZoomLevel").text == "100%", "Demo shows the combat zoom level")
    require(combat.get_node("Footer").text.contains("Shift+arrow: diagonal"),
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
    require(combat.selected_character_id() == 1 and combat.selected_character_cell() == Vector2i(5, 5),
        "Selecting the portrait row selects its party character")
    require(other_selected.get_pixel(1540, 65).r > screenshot.get_pixel(1540, 65).r,
        "Selected portrait row is visibly highlighted")
    require(other_selected.get_pixel(1050, 415).r < 0.25,
        "Off-turn portrait selection shows no movement highlights")
    require(combat.get_node("Log").text.contains("It is not Liora Hallowgrove's turn."),
        "Off-turn selection explains why movement is unavailable")
    var off_turn_key := InputEventKey.new()
    off_turn_key.keycode = KEY_RIGHT
    off_turn_key.pressed = true
    root.push_input(off_turn_key)
    require(combat.selected_character_cell() == Vector2i(5, 5),
        "Arrow keys do not bypass the selected character's turn")
    click.position = Vector2(940, 460)
    root.push_input(click)
    await RenderingServer.frame_post_draw
    screenshot = root.get_texture().get_image()
    require(combat.selected_character_id() == 3 and combat.selected_character_cell() == Vector2i(7, 5),
        "Selecting the combat sprite selects the same active party character")
    require(screenshot.get_pixel(1015, 415).r > screenshot.get_pixel(1140, 415).r,
        "Sprite selection shows the active hero's legal movement squares")
    var active_key := InputEventKey.new()
    active_key.keycode = KEY_RIGHT
    active_key.pressed = true
    root.push_input(active_key)
    require(combat.get_node("Log").text.contains("reaction"),
        "Arrow key starts the selected character's legal move and handles enemy reactions")
    await create_timer(2.0).timeout
    require(combat.selected_character_cell() == Vector2i(8, 5),
        "Arrow key moves the selected character on their turn")
    root.push_input(active_key)
    require(combat.selected_character_cell() == Vector2i(8, 5),
        "Arrow key attacks the adjacent enemy without moving into its square")
    require(combat.get_node("Log").text.contains("Dorian Nightwind -> Kobold"),
        "Arrow key submits a melee attack against the occupied enemy square")
    var output := ProjectSettings.globalize_path("res://../../../build/checks/combat-demo.png")
    require(screenshot.save_png(output) == OK, "Showcase screenshot saves")
    print("Combat demo checks passed: ", output)
    quit(0)
