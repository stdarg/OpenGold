extends SceneTree
## Build a local design preview; extracted artwork stays in ignored user-data.
const Loader = preload("res://scripts/dax_sprite_loader.gd")

func _initialize() -> void:
	var directory: String = OS.get_environment("OPENGOLD_GAME_DIR")
	if directory.is_empty():
		directory = ProjectSettings.get_setting("opengold/game_directory", "")
	var source := ProjectSettings.globalize_path("res://../docs/design/rolf-welcome.html")
	var destination := ProjectSettings.globalize_path("res://../user-data/rolf-welcome.html")
	var fragment := FileAccess.get_file_as_string(source)
	if fragment.is_empty():
		push_error("Could not read the Rolf design source: " + source)
		quit(1)
		return
	var frames: Array = []
	var archive := directory.path_join("SPRIT3.DAX")
	if FileAccess.file_exists(archive):
		var loader := Loader.new()
		var count: int = loader.frame_count(archive, 12)
		if count == 0:
			push_error(loader.error_message)
			quit(1)
			return
		for index in range(count):
			var image: Image = loader.load_frame(archive, 12, index)
			if image == null:
				push_error(loader.error_message)
				quit(1)
				return
			frames.append({"width": image.get_width(), "height": image.get_height(),
				"url": "data:image/png;base64," + Marshalls.raw_to_base64(image.save_png_to_buffer())})
	else:
		print("SPRIT3.DAX unavailable; using the labeled artwork placeholder.")
	var marker := "/* LOCAL_ROLF_SPRITES */ []"
	if fragment.count(marker) != 1:
		push_error("Expected exactly one local-art marker in the design source.")
		quit(1)
		return
	fragment = fragment.replace(marker, JSON.stringify(frames))
	var error := DirAccess.make_dir_recursive_absolute(destination.get_base_dir())
	if error != OK:
		push_error("Could not create local preview directory: %s" % error)
		quit(1)
		return
	var output := FileAccess.open(destination, FileAccess.WRITE)
	if output == null:
		push_error("Could not write local preview: " + destination)
		quit(1)
		return
	output.store_string(fragment)
	output.flush()
	if output.get_error() != OK:
		push_error("Could not finish writing local preview: " + destination)
		quit(1)
		return
	print("Rolf design preview: %s (%d local distance variants)" % [destination, frames.size()])
	quit()
