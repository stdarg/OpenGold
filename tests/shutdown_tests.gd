extends SceneTree

# Use --shutdown-close or --shutdown-key; optional --splash, --shutdown-second,
# --shutdown-dialog, and --shutdown-paused exercise different active views.
func _initialize() -> void:
	call_deferred("run_check")

func settle() -> void:
	for frame in range(6):
		await process_frame

func run_check() -> void:
	var args := OS.get_cmdline_user_args()
	change_scene_to_file("res://scenes/startup.tscn")
	await settle()
	if current_scene == null or not root.has_node("ApplicationLifecycle"):
		push_error("Application startup/lifecycle missing")
		quit(1)
		return
	if args.has("--shutdown-second"):
		var advance := InputEventKey.new()
		advance.keycode = KEY_SPACE
		advance.pressed = true
		Input.parse_input_event(advance)
		await settle()
	var original_scene := current_scene.name
	var original_texture := ""
	if original_scene == "Startup":
		original_texture = current_scene.get_node("Image").texture.resource_path
	var window: Window = root
	if args.has("--shutdown-dialog"):
		window = Window.new()
		current_scene.add_child(window)
		var field := LineEdit.new()
		window.add_child(field)
		window.show()
		field.grab_focus()
	current_scene.tree_exiting.connect(func():
		print("Shutdown check passed: scene teardown after ", "close" if args.has("--shutdown-close") else "Ctrl+X", " from ", original_scene))
	if args.has("--shutdown-paused"):
		paused = true
	if args.has("--shutdown-close"):
		root.close_requested.emit()
		root.close_requested.emit() # repeated requests must be harmless
	else:
		var event := InputEventKey.new()
		event.keycode = KEY_X
		event.ctrl_pressed = true
		event.pressed = true
		if args.has("--shutdown-dialog"):
			window.window_input.emit(event)
		else:
			Input.parse_input_event(event)
		if original_scene == "Startup" and current_scene.get_node("Image").texture.resource_path != original_texture:
			push_error("Ctrl+X advanced the splash")
			quit(1)
			return
	await settle()
	push_error("Shutdown request did not exit")
	quit(1)
