extends SceneTree
var fixtures := ""
var captures := ""
var demo := false
var path := ""
var original: Variant = null
func _init() -> void:
    for arg in OS.get_cmdline_user_args():
        if arg == "--sleep-demo": demo = true
        if arg.begins_with("--sleep-fixtures="): fixtures = arg.trim_prefix("--sleep-fixtures=")
        if arg.begins_with("--sleep-capture="): captures = arg.trim_prefix("--sleep-capture=")
    call_deferred("run_checks")
func cleanup() -> void:
    if original == null:
        if FileAccess.file_exists(path): DirAccess.remove_absolute(path)
    else:
        var file := FileAccess.open(path, FileAccess.WRITE)
        file.store_buffer(original)
func require(ok: bool, message: String) -> void:
    if not ok:
        cleanup(); push_error(message); quit(1); assert(ok, message)
func settle() -> void:
    for frame in range(4): await process_frame
func key(code: Key) -> void:
    for down in [true, false]:
        var event := InputEventKey.new()
        event.keycode = code; event.pressed = down; root.push_input(event)
    await settle()
func load_fixture(name: String) -> void:
    var bytes := FileAccess.get_file_as_bytes(fixtures.path_join(name + ".save"))
    require(not bytes.is_empty(), "Native sleep fixture exists")
    var file := FileAccess.open(path, FileAccess.WRITE)
    file.store_buffer(bytes); file.close()
    current_scene.get_node("Load").pressed.emit(); await settle()
func run_checks() -> void:
    path = ProjectSettings.globalize_path("res://../../user-data/combat.save" if demo else "user://checks/combat.save")
    original = FileAccess.get_file_as_bytes(path) if FileAccess.file_exists(path) else null
    DirAccess.make_dir_recursive_absolute(path.get_base_dir())
    for locale in (["en"] if demo else ["en", "es"]):
        TranslationServer.set_locale(locale)
        change_scene_to_file("res://scenes/combat_demo.tscn"); await settle()
        current_scene.set_process(false)
        await load_fixture("wake")
        var wake: Button = current_scene.get_node("WakeAlly")
        var stand: Button = current_scene.get_node("StandUp")
        require(wake.visible and not wake.disabled, "Legal wake available")
        require(wake.text == ("Wake ally" if locale == "en" else "Despertar aliado"), "Wake translation")
        require(stand.text == ("Stand up" if locale == "en" else "Levantarse"), "Stand translation")
        for size in [Vector2i(1120, 800), Vector2i(1920, 1080)]:
            root.size = size; await settle()
            require(wake.get_rect().end.x <= root.size.x - 300 and stand.get_rect().end.y <= current_scene.get_node("Log").position.y, "Recovery controls fit before party panel and log")
            require(current_scene.get_node("Log").size.y >= 48, "Recovery rows preserve readable log space")
            require(stand.position.y > wake.position.y, "Standing uses approved lower row")
            require(wake.focus_mode == Control.FOCUS_ALL and stand.focus_mode == Control.FOCUS_ALL, "Keyboard focus retained")
            if not captures.is_empty():
                DirAccess.make_dir_recursive_absolute(captures)
                await RenderingServer.frame_post_draw
                root.get_texture().get_image().save_png(captures.path_join(locale + "-" + str(size.x) + ".png"))
        wake.pressed.emit(); await settle(); await key(KEY_ESCAPE)
        require(wake.visible and not wake.disabled, "Escape cancels targeting without spending Action")
        wake.pressed.emit(); await settle()
        var point: Vector2
        if demo:
            var tile := minf((root.size.x - 358.0 - 72.0) / 9.0, (root.size.y - 324.0) / 7.0)
            point = Vector2(24, 116) + Vector2(2.5, 2.5) * tile
        else:
            var canvas: Control = current_scene.get_node("BattlefieldScroll/Canvas")
            point = canvas.get_global_transform_with_canvas() * (Vector2(2.5, 2.5) * (canvas.get_combined_minimum_size().x / 9.0))
        for down in [true, false]:
            var event := InputEventMouseButton.new()
            event.button_index = MOUSE_BUTTON_LEFT; event.position = point; event.global_position = point; event.pressed = down
            root.push_input(event)
        await settle()
        require(not wake.visible, "Clicking sleeping ally wakes it instead of selecting it")
        await load_fixture("stand")
        require(stand.visible and not stand.disabled, "Woken character can stand")
        stand.grab_focus(); await key(KEY_SPACE)
        require(not stand.visible, "Keyboard stand removes Prone")
        await load_fixture("unreachable")
        require(wake.visible and wake.disabled, "Blocked reach disables wake")
        await load_fixture("offturn")
        require(wake.visible and wake.disabled, "Off-turn wake disabled")
        await load_fixture("ground")
        var ground: OptionButton = current_scene.get_node("GroundItem")
        var pickup: Button = current_scene.get_node("PickUp")
        require(ground.visible and ground.item_count == 2 and not pickup.disabled, "Dropped gear uses approved ground selector")
        for size in [Vector2i(1120, 800), Vector2i(1920, 1080)]:
            root.size = size; await settle()
            require(pickup.get_rect().end.x <= root.size.x - 300 and ground.position.y >= stand.position.y, "Ground item controls fit approved row")
            require(current_scene.get_node("Log").size.y >= 48, "Ground items preserve readable log")
            if not captures.is_empty():
                await RenderingServer.frame_post_draw
                root.get_texture().get_image().save_png(captures.path_join("ground-" + locale + "-" + str(size.x) + ".png"))
        require(pickup.text == ("Pick up (interaction)" if locale == "en" else "Recoger (interacción)"), "Pickup displays translated interaction cost")
        pickup.grab_focus(); await key(KEY_SPACE)
        require(ground.item_count == 1 and not pickup.disabled, "Keyboard pickup removes exactly one ground item")
        require(pickup.text == ("Pick up (Action)" if locale == "en" else "Recoger (Acción)"), "Shield recovery displays Action cost")
        await key(KEY_SPACE)
        require(not ground.visible and not pickup.visible, "Shield pickup clears ground row")
        require(not current_scene.get_node("Save").visible and not current_scene.get_node("Load").visible, "No player combat saving")
    cleanup(); print("Natural sleep controls passed"); quit()
