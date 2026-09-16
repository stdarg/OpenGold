extends SceneTree

const CONFIG := "res://settings.cfg"
var scenario := "first"
var real_path := ""
var captures := ""
var args: PackedStringArray

func _initialize() -> void:
	args = OS.get_cmdline_args() + OS.get_cmdline_user_args()
	for arg in args:
		if arg.begins_with("--setup-case="): scenario = arg.trim_prefix("--setup-case=")
		if arg.begins_with("--setup-capture="): captures = arg.trim_prefix("--setup-capture=")
	real_path = OS.get_environment("OPENGOLD_GAME_DIR")
	call_deferred("run_checks")

func require(value: bool, message: String) -> void:
	if not value:
		push_error(message)
		quit(1)
		assert(value, message)

func settle() -> void:
	for frame in range(6): await process_frame

func capture(window: Window, name: String) -> void:
	if captures.is_empty(): return
	DirAccess.make_dir_recursive_absolute(captures)
	await RenderingServer.frame_post_draw
	require(window.get_texture().get_image().save_png(captures.path_join(name + ".png")) == OK, "Capture failed")

func read_config() -> ConfigFile:
	var config := ConfigFile.new()
	config.load(CONFIG)
	return config

func run_checks() -> void:
	require(not real_path.is_empty(), "Set OPENGOLD_GAME_DIR to the local reference installation for this test")
	OS.unset_environment("OPENGOLD_LANG")
	OS.unset_environment("OPENGOLD_GAME_DIR")
	DirAccess.remove_absolute(ProjectSettings.globalize_path(CONFIG))
	var config := ConfigFile.new()
	if scenario != "first" and scenario != "cancel-path" and scenario != "save-error":
		if scenario != "missing-path": config.set_value("game", "path", real_path)
		if scenario != "missing-lang": config.set_value("interface", "language", "es" if scenario == "missing-path" else "en")
		config.set_value("unrelated", "keep", "retained")
		require(config.save(CONFIG) == OK, "Cannot prepare config")
	if scenario == "override":
		OS.set_environment("OPENGOLD_GAME_DIR", real_path)
		OS.set_environment("OPENGOLD_LANG", "es")
	if scenario == "reset":
		OS.set_environment("OPENGOLD_GAME_DIR", "X:/invalid-environment-path")
		OS.set_environment("OPENGOLD_LANG", "es")
	if scenario == "save-error":
		var file := FileAccess.open(CONFIG, FileAccess.WRITE)
		file.store_string("[invalid\n")
		file.close()
	var original := FileAccess.get_file_as_string(CONFIG) if FileAccess.file_exists(CONFIG) else ""
	change_scene_to_file("res://scenes/startup.tscn")
	await settle()
	require(current_scene.name == "Startup", "Startup scene missing")
	var path_dialog: Window = current_scene.get_node("PathDialog")
	var language: Window = current_scene.get_node("LanguageDialog")
	if scenario in ["valid", "override"]:
		require(not path_dialog.visible and not language.visible, "Complete config should skip setup")
		require(current_scene.get_node("Text").texture != null, "Splash did not start")
		require(TranslationServer.get_locale() == ("es" if scenario == "override" else "en"), "Language precedence incorrect")
		require(FileAccess.get_file_as_string(CONFIG) == original, "Override rewrote config")
	elif scenario in ["missing-lang", "cancel-lang"]:
		require(not path_dialog.visible and language.visible, "Should ask only for missing/reset language")
		if scenario == "cancel-lang":
			current_scene.tree_exiting.connect(func():
				require(FileAccess.get_file_as_string(CONFIG) == original, "Cancel changed config")
				print("Setup cancel-language passed."))
			language.get_node("Cancel").pressed.emit()
			await settle()
			require(false, "Cancel did not exit")
			return
		await choose_spanish(language)
	else:
		require(path_dialog.visible and not language.visible, "Path must be prompted before language")
		await capture(path_dialog, "game-path-" + scenario)
		if scenario == "cancel-path":
			current_scene.tree_exiting.connect(func():
				require(not FileAccess.file_exists(CONFIG), "Cancel created config")
				print("Setup cancel-path passed."))
			path_dialog.get_node("Cancel").pressed.emit()
			await settle()
			require(false, "Cancel did not exit")
			return
		if scenario == "reset":
			require(FileAccess.get_file_as_string(CONFIG) == original, "Reset erased saved choices before confirmation")
			require(TranslationServer.get_locale() == "en", "Reset language did not supersede environment override")
		if scenario == "first":
			path_dialog.get_node("Path").text = "X:/not-a-game-folder"
			path_dialog.get_node("Continue").pressed.emit()
			await settle()
			require(path_dialog.visible and not path_dialog.get_node("Status").text.is_empty(), "Invalid folder was accepted")
			path_dialog.get_node("Browse").pressed.emit()
			await settle()
			var browser: FileDialog = path_dialog.get_node("BrowseDialog")
			require(browser.visible and browser.file_mode == FileDialog.FILE_MODE_OPEN_DIR, "Browse must select a folder")
			browser.dir_selected.emit(real_path)
			browser.hide()
			require(path_dialog.get_node("Path").text == real_path, "Browse result was not applied")
		var selected := real_path
		if scenario in ["mismatch", "warning-quit", "missing-file"]:
			selected = ProjectSettings.globalize_path("user://synthetic-files-" + scenario)
			DirAccess.make_dir_recursive_absolute(selected)
			var manifest: Dictionary = JSON.parse_string(FileAccess.get_file_as_string("res://config/por-pc13-md5.json"))
			for name in manifest.files:
				var fixture := FileAccess.open(selected.path_join(name), FileAccess.WRITE)
				fixture.store_string("Synthetic test data; not original game content.")
				fixture.close()
			if scenario == "missing-file": DirAccess.remove_absolute(selected.path_join("START.EXE"))
		path_dialog.get_node("Path").text = selected
		path_dialog.get_node("Continue").pressed.emit()
		await settle()
		if scenario == "save-error":
			require(path_dialog.visible and path_dialog.get_node("Status").text.contains("Cannot save"), "Malformed config must report a save error")
			require(FileAccess.get_file_as_string(CONFIG) == original, "Malformed config was overwritten")
		elif scenario == "missing-file":
			require(path_dialog.visible and path_dialog.get_node("Status").text.contains("START.EXE"), "Missing required file was not reported")
			require(FileAccess.get_file_as_string(CONFIG) == original, "Invalid path was saved")
		else:
			if scenario in ["mismatch", "warning-quit"]:
				var warning: Window = current_scene.get_node("ChecksumWarning")
				require(warning.visible and warning.get_node("Files").text.contains("START.EXE"), "MD5 mismatch warning missing")
				require(FileAccess.get_file_as_string(CONFIG) == original, "Unaccepted mismatch path was saved")
				await capture(warning, "checksum-warning")
				if scenario == "warning-quit":
					current_scene.tree_exiting.connect(func():
						require(FileAccess.get_file_as_string(CONFIG) == original, "Quit changed saved path")
						print("Setup checksum-quit passed."))
					warning.get_node("Quit").pressed.emit()
					await settle()
					require(false, "Warning Quit did not exit")
					return
				warning.get_node("Continue").pressed.emit()
				await settle()
			require(read_config().get_value("game", "path") == selected, "Confirmed path was not saved")
			if scenario in ["first", "reset"]:
				require(language.visible, "Language must follow confirmed path")
				await choose_spanish(language)
			else:
				require(not language.visible, "Existing language was prompted again")
				require(current_scene.get_node("Text").texture != null, "Path confirmation did not start splash")
	if scenario not in ["first", "save-error"]:
		require(read_config().get_value("unrelated", "keep") == "retained", "Unrelated setting lost")
	print("Setup config passed: ", scenario)
	quit(0)

func choose_spanish(dialog: Window) -> void:
	var choices: ItemList = dialog.get_node("Choices")
	choices.select(1)
	choices.item_selected.emit(1)
	require(dialog.get_node("Continue").text == "Continuar", "Language preview failed")
	dialog.get_node("Continue").pressed.emit()
	await settle()
	require(read_config().get_value("interface", "language") == "es", "Spanish was not saved")
	require(current_scene.get_node("Text").texture.resource_path.ends_with("OpenGoldBoxEngineLettering.es.png"), "Confirmation skipped first Spanish splash")
