extends SceneTree

# Run using Godot against src/OpenGoldBox/godot with --script and optionally --splash.
var shared_texture: Texture2D
var background_pixels := PackedByteArray()

func _initialize() -> void:
	call_deferred("run_checks")

func require(condition: bool, message: String) -> bool:
	if not condition:
		push_error(message)
		quit(1)
	return condition

func settle() -> void:
	# No-flag startup performs a second, deferred scene change from _ready.
	for frame in range(6):
		await process_frame

func restart_startup() -> void:
	change_scene_to_file("res://scenes/startup.tscn")
	await settle()

func is_splash(index: int) -> bool:
	if not require(current_scene != null and current_scene.name == "Startup", "Expected splash scene"):
		return false
	var image: TextureRect = current_scene.get_node("Image")
	var filename := "OpenGoldBoxSplashBackground.png"
	if shared_texture == null:
		shared_texture = image.texture
	if not require(image.texture == shared_texture, "Both splash screens must use the same background texture"): return false
	var lettering: TextureRect = current_scene.get_node("Text")
	var expected := "OpenGoldBoxEngineLettering.png" if index == 0 else "OpenGoldBoxGameLettering.png"
	if not require(lettering.texture != null and lettering.texture.resource_path.ends_with(expected), "Wrong splash lettering"): return false
	if not require(lettering.stretch_mode == TextureRect.STRETCH_KEEP_ASPECT_CENTERED, "Lettering must preserve its proportions"): return false
	var pixels := lettering.texture.get_image()
	if not require(pixels.detect_alpha() != Image.ALPHA_NONE and pixels.get_pixel(0, 0).a == 0, "Lettering must have a transparent background"): return false
	return require(image.texture != null and image.texture.resource_path.ends_with(filename), "Wrong splash image") and require(image.stretch_mode == TextureRect.STRETCH_KEEP_ASPECT_CENTERED, "Splash must preserve aspect ratio") and require(current_scene.get_node("Background").color == Color.BLACK, "Letterbox must be black")

func key(code: Key, pressed := true, echo := false) -> void:
	var event := InputEventKey.new()
	event.keycode = code
	event.pressed = pressed
	event.echo = echo
	Input.parse_input_event(event)
	await settle()

func capture(filename: String) -> void:
	for arg in OS.get_cmdline_user_args():
		if arg.begins_with("--splash-capture-dir="):
			var directory := arg.trim_prefix("--splash-capture-dir=")
			DirAccess.make_dir_recursive_absolute(directory)
			await RenderingServer.frame_post_draw
			var image := root.get_texture().get_image()
			require(image != null and image.save_png(directory.path_join(filename)) == OK, "Splash capture failed")
			# Compare the actual rendered backdrop, including sampling and placement.
			current_scene.get_node("Text").hide()
			await settle()
			await RenderingServer.frame_post_draw
			var pixels := root.get_texture().get_image().get_data()
			if background_pixels.is_empty():
				background_pixels = pixels
			else:
				require(pixels == background_pixels, "Splash backgrounds differ at the pixel level")
				print("Splash backgrounds are pixel-identical")
			current_scene.get_node("Text").show()
			await settle()

func run_checks() -> void:
	for setting in ["viewport_width", "window_width_override"]:
		if not require(ProjectSettings.get_setting("display/window/size/" + setting) == 1920, "Game width must default to 1920"): return
	for setting in ["viewport_height", "window_height_override"]:
		if not require(ProjectSettings.get_setting("display/window/size/" + setting) == 1080, "Game height must default to 1080"): return
	await restart_startup()
	var enabled := OS.get_cmdline_args().has("--splash") or OS.get_cmdline_user_args().has("--splash")
	if not enabled:
		if not require(current_scene != null and current_scene.name == "CharacterCreation", "Without --splash, character creation must open"):
			return
		print("Startup check passed: no flag skips both splashes")
		quit(0)
		return
	if not is_splash(0): return
	await capture("splash-first.png")
	await key(KEY_SPACE, false)
	if not is_splash(0): return
	var mouse := InputEventMouseButton.new()
	mouse.button_index = MOUSE_BUTTON_LEFT
	mouse.pressed = true
	Input.parse_input_event(mouse)
	await settle()
	if not is_splash(0): return
	await key(KEY_SPACE)
	if not is_splash(1): return
	await capture("splash-second.png")
	await key(KEY_SPACE, true, true)
	if not is_splash(1): return
	await key(KEY_SPACE, false)
	if not is_splash(1): return
	await key(KEY_ENTER)
	if not require(current_scene.name == "CharacterCreation", "Second key must open character creation"): return
	await restart_startup()
	await key(KEY_ESCAPE)
	if not require(current_scene.name == "CharacterCreation", "Escape must skip first splash"): return
	await restart_startup()
	await key(KEY_A)
	if not is_splash(1): return
	await key(KEY_ESCAPE)
	if not require(current_scene.name == "CharacterCreation", "Escape must skip second splash"): return
	print("Startup check passed: two splashes, key progression, ignored release/repeat/mouse, Escape from either screen")
	quit(0)
