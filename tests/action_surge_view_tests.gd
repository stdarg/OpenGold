extends SceneTree
var originals := {}
var path := ""
var fixtures := ""
func _initialize() -> void:
    for arg in OS.get_cmdline_user_args():
        if arg.begins_with("--surge-fixtures="): fixtures = arg.trim_prefix("--surge-fixtures=")
    call_deferred("run_checks")
func restore_files() -> void:
    for name in originals:
        if originals[name] == null:
            if FileAccess.file_exists(name): DirAccess.remove_absolute(name)
        else:
            var file := FileAccess.open(name, FileAccess.WRITE)
            file.store_buffer(originals[name]); file.close()
    originals.clear()
func require(ok: bool, message: String) -> void:
    if not ok:
        restore_files()
        push_error(message)
        quit(1)
        assert(ok, message)
func settle() -> void:
    for frame in range(4): await process_frame
func key(code: Key) -> void:
    for down in [true, false]:
        var event := InputEventKey.new()
        event.keycode = code; event.pressed = down
        root.push_input(event)
    await settle()
func choose(label: String) -> void:
    for attempt in range(20):
        await key(KEY_A)
        if current_scene.get_node("Prompt").text.contains(label): return
    require(false, "Existing keyboard cycle lacks " + label)
func load_fixture(name: String) -> void:
    var bytes := FileAccess.get_file_as_bytes(fixtures.path_join(name + ".save"))
    require(not bytes.is_empty(), "Native fixture exists")
    var file := FileAccess.open(path, FileAccess.WRITE)
    file.store_buffer(bytes); file.close()
    current_scene.get_node("Load").pressed.emit()
    await settle()
func run_checks() -> void:
    path = ProjectSettings.globalize_path("user://checks/combat.save")
    for suffix in ["", ".bak"]:
        originals[path + suffix] = FileAccess.get_file_as_bytes(path + suffix) if FileAccess.file_exists(path + suffix) else null
    DirAccess.make_dir_recursive_absolute(path.get_base_dir())
    change_scene_to_file("res://scenes/combat_demo.tscn")
    await settle()
    current_scene.set_process(false)
    require(not current_scene.get_node("Save").visible and not current_scene.get_node("Load").visible, "No player combat saving")
    await load_fixture("available")
    await choose("Action Surge")
    await key(KEY_SPACE)
    require(current_scene.get_node("Log").get_parsed_text().contains("uses Action Surge"), "Existing keyboard action activates the feature")
    await choose("Dash")
    await key(KEY_SPACE)
    require(current_scene.get_node("Log").get_parsed_text().contains("Move 60 ft | Action ready"), "Extra Dash retains ordinary action")
    await choose("Dash")
    await key(KEY_SPACE)
    require(current_scene.get_node("Log").get_parsed_text().contains("Move 90 ft | Action spent"), "Second Dash spends the ordinary action")
    var prompts := ""
    for i in range(18):
        await key(KEY_A)
        prompts += current_scene.get_node("Prompt").text + "\n"
    require(not prompts.contains("Action Surge") and not prompts.contains("Dash"), "Exhausted actions leave the cycle")
    restore_files()
    print("Action Surge existing keyboard checks passed")
    quit(0)
