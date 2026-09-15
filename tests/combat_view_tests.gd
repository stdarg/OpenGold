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
    change_scene_to_file("res://scenes/combat_demo.tscn")
    await settle()
    var combat := current_scene
    var scroll: ScrollContainer = combat.get_node("BattlefieldScroll")
    var canvas: Control = scroll.get_node("Canvas")
    require(canvas.size.is_equal_approx(scroll.size * 3), "Battlefield must be exactly 3x the original fit size")
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
    mouse_button(combat.get_node("Title").global_position, MOUSE_BUTTON_WHEEL_UP, true)
    require(Vector2(scroll.scroll_horizontal, scroll.scroll_vertical) == before, "Wheel outside battlefield must not pan it")
    root.size = Vector2i(1120, 800)
    await settle()
    require(canvas.size.is_equal_approx(scroll.size * 3), "Resize must preserve 3x magnification")
    if OS.get_cmdline_user_args().has("--capture"):
        root.size = Vector2i(1280, 900)
        await settle()
        await RenderingServer.frame_post_draw
        var path := ProjectSettings.globalize_path("res://../../../build/checks/combat-zoom.png")
        DirAccess.make_dir_recursive_absolute(path.get_base_dir())
        require(root.get_texture().get_image().save_png(path) == OK, "Capture failed")
    print("Game combat view checks passed: 3x scale, centering, drag, wheel, bounds, resize")
    quit(0)
