extends SceneTree

var saved_files := {}

func _initialize() -> void:
    call_deferred("run_checks")

func restore_files() -> void:
    for path in saved_files:
        if saved_files[path] == null:
            if FileAccess.file_exists(path):
                DirAccess.remove_absolute(path)
        else:
            var file := FileAccess.open(path, FileAccess.WRITE)
            if file:
                file.store_buffer(saved_files[path])
                file.close()
    saved_files.clear()

func require(ok: bool, message: String) -> void:
    if not ok:
        restore_files()
        push_error(message)
        quit(1)
        assert(ok, message)

func key(code: Key) -> void:
    var event := InputEventKey.new()
    event.keycode = code
    event.pressed = true
    root.push_input(event)

func run_checks() -> void:
    var path := ProjectSettings.globalize_path("user://checks/combat.save")
    for suffix in ["", ".bak"]:
        saved_files[path + suffix] = FileAccess.get_file_as_bytes(path + suffix) if FileAccess.file_exists(path + suffix) else null
    DirAccess.make_dir_recursive_absolute(path.get_base_dir())
    change_scene_to_file("res://scenes/combat_demo.tscn")
    for frame in range(4):
        await process_frame
    var combat := current_scene
    combat.set_process(false)
    for fixture in ["facing", "movement"]:
        var bytes := FileAccess.get_file_as_bytes("res://../../../tests/fixtures/combat-v5-" + fixture + ".save")
        require(not bytes.is_empty(), "Frozen previous-module fixture is readable")
        var file := FileAccess.open(path, FileAccess.WRITE)
        require(file != null, "Prepare checkpoint for the real Load control")
        file.store_buffer(bytes)
        file.close()
        combat.get_node("Load").pressed.emit()
        if fixture == "facing":
            require(combat.sprite_facing_left(1), "Migrated sprite still faces its previous attack target")
            require(combat.get_node("Turn").text.contains("Mover turn"), "Canceled facing queue resumes the attacker")
            require(not combat.get_node("End").disabled and combat.get_node("React").disabled,
                "No obsolete reaction blocks the resumed turn")
            require(combat.get_node("Melee").disabled and combat.get_node("SecondWind").disabled,
                "Migration preserves spent action and recovery")
            key(KEY_DOWN)
            require(combat.selected_character_cell() == Vector2i(2, 4), "Remaining movement works through real input after migration")
            require(combat.sprite_facing_left(1), "Movement does not reset presentation facing")
        else:
            require(combat.get_node("Turn").text.contains("Second guard reaction"), "Actual movement queue retains its next reactor")
            require(combat.selected_character_cell() == Vector2i(2, 2), "Mover still waits before leaving reach")
            var before: String = combat.get_node("Turn").text
            key(KEY_ENTER)
            require(combat.get_node("Turn").text == before, "Keyboard cannot bypass the enemy's pending reaction")
        var expected_turn: String = combat.get_node("Turn").text
        var expected_roster: String = combat.get_node("Roster").text
        combat.get_node("Save").pressed.emit()
        require(combat.get_node("Prompt").text.contains("saved"), "Migrated checkpoint saves through existing controls")
        require(FileAccess.get_file_as_string(path).begins_with("OGCOMBAT 6 "), "Game writes the new checkpoint format")
        combat.get_node("Load").pressed.emit()
        require(combat.get_node("Turn").text == expected_turn and combat.get_node("Roster").text == expected_roster,
            "Subsequent reload preserves turn, resources and pending movement")
    restore_files()
    print("Opportunity view checks passed: migrated facing, remaining movement, pending reaction, save/load")
    quit(0)
