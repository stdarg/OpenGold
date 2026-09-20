extends SceneTree

# Exercise the game combat canvas through real Godot input and container layout.
func _initialize() -> void:
    call_deferred("run_checks")

func require(ok: bool, message: String) -> void:
    if not ok:
        push_error(message)
        quit(1)
        assert(ok, message)

func settle() -> void:
    for frame in range(4):
        await process_frame

func mouse_button(position: Vector2, button: MouseButton, pressed: bool, shift := false) -> void:
    var event := InputEventMouseButton.new()
    event.position = position
    event.button_index = button
    event.pressed = pressed
    event.shift_pressed = shift
    root.push_input(event)

func run_checks() -> void:
    var config_path := ProjectSettings.globalize_path("res://settings.cfg")
    var had_config := FileAccess.file_exists(config_path)
    var original_config := FileAccess.get_file_as_bytes(config_path) if had_config else PackedByteArray()
    if had_config:
        require(DirAccess.remove_absolute(config_path) == OK, "Isolate combat zoom test config")
    root.size = Vector2i(1920, 1080)
    change_scene_to_file("res://scenes/combat_demo.tscn")
    await settle()
    var combat := current_scene
    var scroll: ScrollContainer = combat.get_node("BattlefieldScroll")
    var canvas: Control = scroll.get_node("Canvas")
    require(ProjectSettings.get_setting("opengold/combat_zoom") == 200, "Combat zoom defaults to 200 in project config")
    require(scroll.size.x > combat.size.x * 0.7, "Battlefield viewport fills the left side")
    require(scroll.position.y == 16, "Battlefield begins at the top of the screen")
    require(not combat.has_node("Title") and not combat.has_node("Subtitle"),
        "Combat header and instruction text are removed from the shared scene")
    require(combat.get_node("ZoomLevel").text == "200%", "Zoom readout shows the initial level")
    var base_tile := minf(scroll.size.x / 12.0, scroll.size.y / 9.0)
    require(canvas.custom_minimum_size.is_equal_approx(Vector2(12, 9) * base_tile * 2),
        "Battlefield uses configured 200% zoom")
    require(scroll.clip_contents, "Magnified battlefield must be clipped")
    require(scroll.scroll_horizontal > 0 or scroll.scroll_vertical > 0, "Initial actor must be centered")
    scroll.scroll_horizontal = 120
    scroll.scroll_vertical = 120
    await settle()
    var point := scroll.global_position + scroll.size / 2
    mouse_button(point, MOUSE_BUTTON_MIDDLE, true)
    var motion := InputEventMouseMotion.new()
    motion.position = point + Vector2(40, 30)
    motion.relative = Vector2(40, 30)
    motion.button_mask = MOUSE_BUTTON_MASK_MIDDLE
    root.push_input(motion)
    mouse_button(motion.position, MOUSE_BUTTON_MIDDLE, false)
    require(scroll.scroll_horizontal == 80 and scroll.scroll_vertical == 90, "Middle drag must pan both axes")
    await settle()
    require(scroll.scroll_horizontal == 80 and scroll.scroll_vertical == 90, "Manual pan must persist during the same turn")
    mouse_button(point, MOUSE_BUTTON_WHEEL_DOWN, true)
    mouse_button(point, MOUSE_BUTTON_WHEEL_DOWN, false)
    require(scroll.scroll_vertical > 90, "Mouse wheel must scroll vertically")
    mouse_button(point, MOUSE_BUTTON_WHEEL_DOWN, true, true)
    mouse_button(point, MOUSE_BUTTON_WHEEL_DOWN, false, true)
    require(scroll.scroll_horizontal > 80, "Shift-wheel must scroll horizontally")
    var hbar := scroll.get_h_scroll_bar()
    require(hbar.focus_mode == Control.FOCUS_ALL, "Horizontal scrollbar must support keyboard focus")
    hbar.grab_focus()
    var horizontal_before := scroll.scroll_horizontal
    var key := InputEventKey.new()
    key.keycode = KEY_RIGHT
    key.pressed = true
    root.push_input(key)
    require(scroll.scroll_horizontal > horizontal_before, "Focused scrollbar must support arrow keys")
    scroll.scroll_horizontal = 100000
    scroll.scroll_vertical = 100000
    await settle()
    require(scroll.scroll_horizontal <= canvas.size.x and scroll.scroll_vertical <= canvas.size.y, "Scrolling must clamp to battlefield bounds")
    var before := Vector2(scroll.scroll_horizontal, scroll.scroll_vertical)
    mouse_button(combat.get_node("Turn").global_position, MOUSE_BUTTON_WHEEL_UP, true)
    require(Vector2(scroll.scroll_horizontal, scroll.scroll_vertical) == before, "Wheel outside battlefield must not pan it")
    root.size = Vector2i(1120, 800)
    await settle()
    base_tile = minf(scroll.size.x / 12.0, scroll.size.y / 9.0)
    require(canvas.custom_minimum_size.is_equal_approx(Vector2(12, 9) * base_tile * 2),
        "Resize preserves configured magnification")
    combat.get_node("ZoomIn10").pressed.emit()
    await settle()
    require(canvas.custom_minimum_size.is_equal_approx(Vector2(12, 9) * base_tile * 2.1),
        "+10% button scales the shared battlefield")
    require(combat.get_node("ZoomLevel").text == "210%", "Zoom readout follows the +10% control")
    combat.get_node("ZoomIn100").pressed.emit()
    await settle()
    require(canvas.custom_minimum_size.is_equal_approx(Vector2(12, 9) * base_tile * 3.1),
        "+100% button scales the shared battlefield")
    require(combat.get_node("ZoomLevel").text == "310%", "Zoom readout follows the +100% control")
    combat.get_node("ZoomOut100").pressed.emit()
    combat.get_node("ZoomOut10").pressed.emit()
    await settle()
    require(canvas.custom_minimum_size.is_equal_approx(Vector2(12, 9) * base_tile * 2),
        "Zoom buttons reverse to the 200% default")
    require(scroll.scroll_horizontal > 0 or scroll.scroll_vertical > 0,
        "Zoom recenters the active character")
    if OS.get_cmdline_user_args().has("--capture"):
        root.size = Vector2i(1920, 1080)
        await settle()
        await RenderingServer.frame_post_draw
        var path := ProjectSettings.globalize_path("res://../../../build/checks/combat-zoom.png")
        DirAccess.make_dir_recursive_absolute(path.get_base_dir())
        require(root.get_texture().get_image().save_png(path) == OK, "Capture failed")
    var config := ConfigFile.new()
    config.set_value("combat", "combat_zoom", 300)
    require(config.save(config_path) == OK, "Prepare portable combat zoom setting")
    change_scene_to_file("res://scenes/combat_demo.tscn")
    await settle()
    var alternate_scroll: ScrollContainer = current_scene.get_node("BattlefieldScroll")
    var alternate_canvas: Control = alternate_scroll.get_node("Canvas")
    base_tile = minf(alternate_scroll.size.x / 12.0, alternate_scroll.size.y / 9.0)
    require(alternate_canvas.custom_minimum_size.is_equal_approx(Vector2(12, 9) * base_tile * 3),
        "Combat reads 300% from settings.cfg")
    if had_config:
        var file := FileAccess.open(config_path, FileAccess.WRITE)
        require(file != null, "Restore original config")
        file.store_buffer(original_config)
        file.close()
    else:
        require(DirAccess.remove_absolute(config_path) == OK, "Remove temporary config")
    print("Game combat view checks passed: shared zoom buttons, centering, viewport, drag, wheel, bounds, resize")
    quit(0)
