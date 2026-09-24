extends SceneTree

var legacy := false
var fixtures := ""
var captures := ""
var originals := {}
var save_path := ""

func _initialize() -> void:
    for arg in OS.get_cmdline_user_args():
        if arg == "--legacy": legacy = true
        if arg.begins_with("--savage-fixtures="): fixtures = arg.trim_prefix("--savage-fixtures=")
        if arg.begins_with("--savage-capture="): captures = arg.trim_prefix("--savage-capture=")
    call_deferred("run_checks")

func restore_files() -> void:
    for path in originals:
        if originals[path] == null:
            if FileAccess.file_exists(path): DirAccess.remove_absolute(path)
        else:
            var file := FileAccess.open(path, FileAccess.WRITE)
            file.store_buffer(originals[path]); file.close()
    originals.clear()

func require(ok: bool, message: String) -> void:
    if not ok:
        restore_files()
        push_error(message)
        quit(1)
        assert(ok, message)

func settle() -> void:
    for frame in range(4): await process_frame

func load_fixture(which: String) -> void:
    var bytes := FileAccess.get_file_as_bytes(fixtures.path_join("savage-" + which + ".save"))
    require(not bytes.is_empty(), "Normally created Soldier fixture exists")
    var file := FileAccess.open(save_path, FileAccess.WRITE)
    file.store_buffer(bytes); file.close()
    current_scene.get_node("Load").pressed.emit()
    await settle()

func key_press(window: Window, button: Button) -> void:
    button.grab_focus()
    await settle()
    var down := InputEventKey.new()
    down.keycode = KEY_ENTER; down.pressed = true
    window.push_input(down)
    var up := InputEventKey.new()
    up.keycode = KEY_ENTER
    window.push_input(up)
    await settle()

func capture(name: String, window: Window) -> void:
    if captures.is_empty(): return
    DirAccess.make_dir_recursive_absolute(captures)
    await RenderingServer.frame_post_draw
    require(window.get_texture().get_image().save_png(captures.path_join(name + ".png")) == OK, "Rendered decision captured")

func run_checks() -> void:
    save_path = ProjectSettings.globalize_path("res://../../user-data/combat.save" if legacy else "user://checks/combat.save")
    for suffix in ["", ".bak"]:
        originals[save_path + suffix] = FileAccess.get_file_as_bytes(save_path + suffix) if FileAccess.file_exists(save_path + suffix) else null
    DirAccess.make_dir_recursive_absolute(save_path.get_base_dir())
    change_scene_to_file("res://scenes/combat_demo.tscn")
    await settle()
    var combat := current_scene
    combat.set_process(false)
    var modal: Window = combat.get_node("SavageAttacker")
    require(not combat.get_node("Save").visible and not combat.get_node("Load").visible, "No player combat save controls")
    await load_fixture("first")
    require(modal.visible and modal.exclusive and modal.get_node("Use").has_focus(), "Exclusive decision receives keyboard focus")
    require(modal.get_node("Text").text.contains("2d6") and modal.get_node("Text").text.contains("9"), "First roll shows weapon dice and damage")
    for path in ["End", "React", "Decline", "Dash"]:
        require(combat.get_node(path).disabled, "Other action cannot bypass decision: " + path)
    for name in ["Use", "Skip", "First", "Second"]:
        var button: Button = modal.get_node(name)
        require(button.focus_mode == Control.FOCUS_ALL and button.get_theme_stylebox("normal") != null and button.get_theme_stylebox("focus") != null, "Visible button styling and keyboard focus")
    for dimensions in [Vector2i(1920, 1080), Vector2i(1120, 800)]:
        root.size = dimensions
        modal.popup_centered()
        await settle()
        require(modal.size.x <= dimensions.x and modal.size.y <= dimensions.y, "Dialog fits supported window sizes")
        await capture("savage-first-" + str(dimensions.x), modal)
    await key_press(modal, modal.get_node("Use"))
    require(modal.visible and not modal.get_node("Use").visible and modal.get_node("First").has_focus(), "Use advances to second-stage choice with fresh focus")
    require(modal.get_node("First").text.contains("9") and modal.get_node("Second").text.contains("10"), "Both damage results appear on real buttons")
    require(combat.get_node("Roster").text.replace(" ", "").contains("28/28"), "Target damage waits for final choice")
    combat.get_node("Save").pressed.emit()
    var pending := FileAccess.get_file_as_string(save_path)
    combat.get_node("Load").pressed.emit()
    await settle()
    combat.get_node("Save").pressed.emit()
    require(FileAccess.get_file_as_string(save_path) == pending and modal.get_node("First").visible, "Internal checkpoint restores second-stage choice without reroll")
    await capture("savage-second", modal)
    await key_press(modal, modal.get_node("First"))
    require(not modal.visible and combat.get_node("Roster").text.replace(" ", "").contains("19/28"), "Keyboard can deliberately keep lower first damage")
    await load_fixture("second")
    modal.get_node("Second").pressed.emit()
    await settle()
    require(not modal.visible and combat.get_node("Roster").text.replace(" ", "").contains("18/28"), "Second result applies once")
    await load_fixture("first")
    await key_press(modal, modal.get_node("Skip"))
    require(not modal.visible and combat.get_node("Roster").text.replace(" ", "").contains("19/28"), "Decline applies first damage and returns control")
    restore_files()
    print("Savage view checks passed: both decisions, keyboard focus, pending damage, internal continuation and supported sizes")
    quit(0)
