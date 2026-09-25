extends SceneTree
var originals := {}
var save_path := ""
var fixtures := ""
var captures := ""
var demo := false
func _initialize() -> void:
    for arg in OS.get_cmdline_user_args():
        if arg.begins_with("--rogue-fixtures="): fixtures = arg.trim_prefix("--rogue-fixtures=")
        if arg.begins_with("--rogue-captures="): captures = arg.trim_prefix("--rogue-captures=")
        if arg == "--rogue-demo": demo = true
    call_deferred("run_checks")
func restore_files() -> void:
    for name in originals:
        if originals[name] == null:
            if FileAccess.file_exists(name): DirAccess.remove_absolute(name)
        else:
            var file := FileAccess.open(name, FileAccess.WRITE)
            file.store_buffer(originals[name]); file.close()
func require(ok: bool, message: String) -> void:
    if not ok:
        restore_files(); push_error(message); quit(1); assert(ok, message)
func settle() -> void:
    for frame in range(4): await process_frame
func load_fixture(name: String) -> void:
    var bytes := FileAccess.get_file_as_bytes(fixtures.path_join(name + ".save"))
    require(not bytes.is_empty(), "Current native fixture exists")
    var file := FileAccess.open(save_path, FileAccess.WRITE)
    file.store_buffer(bytes); file.close()
    current_scene.get_node("Load").pressed.emit()
    await settle()
func press(window: Window, button: Button) -> void:
    button.grab_focus(); await settle()
    for down in [true, false]:
        var event := InputEventKey.new()
        event.keycode = KEY_ENTER; event.pressed = down
        window.push_input(event)
    await settle()
func capture(name: String, window: Window) -> void:
    if captures.is_empty(): return
    DirAccess.make_dir_recursive_absolute(captures)
    await RenderingServer.frame_post_draw
    require(window.get_texture().get_image().save_png(captures.path_join(name + ".png")) == OK, "Decision render saved")
func run_checks() -> void:
    save_path = ProjectSettings.globalize_path("res://../../user-data/combat.save" if demo else "user://checks/combat.save")
    for suffix in ["", ".bak"]:
        originals[save_path + suffix] = FileAccess.get_file_as_bytes(save_path + suffix) if FileAccess.file_exists(save_path + suffix) else null
    DirAccess.make_dir_recursive_absolute(save_path.get_base_dir())
    change_scene_to_file("res://scenes/combat_demo.tscn"); await settle()
    current_scene.set_process(false)
    require(not current_scene.get_node("Save").visible and not current_scene.get_node("Load").visible, "No player combat-saving controls")
    var locales := ["en"] if demo else ["en", "es"]
    for locale in locales:
        TranslationServer.set_locale(locale)
        for size in [Vector2i(1120, 800), Vector2i(1920, 1080)]:
            root.size = size; await settle()
            for level in range(1, 5):
                await load_fixture("sneak-level" + str(level))
                var popup: Window = current_scene.get_node("SneakAttack")
                require(popup.visible and popup.size == Vector2i(700, 380), "Approved centered Sneak dialog: visible=" + str(popup.visible) + " size=" + str(popup.size) + " prompt=" + current_scene.get_node("Prompt").text + " log=" + current_scene.get_node("Log").text)
                require(not current_scene.get_node("SavageAttacker").visible, "Sneak precedes Savage")
                var use: Button = popup.get_node("Use")
                var skip: Button = popup.get_node("Skip")
                require(use.size.x == 652 and skip.size.x == 652 and skip.position.y > use.position.y + use.size.y, "Stacked buttons have distinct full-width bounds")
                require(use.focus_mode == Control.FOCUS_ALL and skip.focus_mode == Control.FOCUS_ALL, "Both choices support keyboard focus")
                await capture(locale + "-" + str(size.x) + "-sneak-" + str(level), popup)
                await press(popup, use)
                var savage: Window = current_scene.get_node("SavageAttacker")
                require(not popup.visible and savage.visible, "Keyboard use advances to weapon-only Savage decision")
                require("Ataque furtivo" in savage.get_node("Text").text if locale == "es" else "Sneak Attack" in savage.get_node("Text").text, "Retained extra damage explained beside weapon rolls")
                current_scene.get_node("Save").pressed.emit(); await settle()
                require(FileAccess.get_file_as_bytes(save_path) == FileAccess.get_file_as_bytes(fixtures.path_join("savage-extra-level" + str(level) + ".save")), "UI use matches native continuation exactly")
                await capture(locale + "-" + str(size.x) + "-savage-" + str(level), savage)
                await press(savage, savage.get_node("Skip"))
            if not demo:
                await load_fixture("aim-available")
                var choices: OptionButton = current_scene.get_node("CunningAction")
                var use_bonus: Button = current_scene.get_node("UseCunningAction")
                require(choices.visible and choices.item_count == 3 and not choices.is_item_disabled(2), "Steady Aim joins existing Bonus Action choices")
                choices.select(2); choices.item_selected.emit(2); await settle()
                require(not use_bonus.disabled, "Selected legal Steady Aim can activate")
                await capture(locale + "-" + str(size.x) + "-aim", root)
                await press(root, use_bonus)
                require(use_bonus.disabled and current_scene.get_node("Move").disabled, "Aim spends Bonus Action and blocks movement")
                current_scene.get_node("Save").pressed.emit(); await settle()
                require(FileAccess.get_file_as_bytes(save_path) == FileAccess.get_file_as_bytes(fixtures.path_join("aim-spent.save")), "UI Aim matches native spending exactly")
    await load_fixture("sneak-level1")
    var popup: Window = current_scene.get_node("SneakAttack")
    popup.close_requested.emit(); await settle()
    require(not popup.visible and current_scene.get_node("SavageAttacker").visible, "Escape/window close declines Sneak without canceling the hit")
    restore_files(); print("Rogue attack UI checks passed", " (demo Sneak only)" if demo else ""); quit(0)
