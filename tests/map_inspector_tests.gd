extends SceneTree

func _initialize() -> void:
	call_deferred("_run")

func _run() -> void:
	var scene = load("res://scenes/map_inspector.tscn").instantiate()
	root.add_child(scene)
	await process_frame
	# Native exporter integration when a game installation is configured.
	if scene.maps.is_empty():
		print("Installed-map check skipped: ", scene.info.text)
	else:
		for i in range(scene.maps.size()):
			scene._select_map(i)
			assert(scene.maps[i].cells.size() == 256)
			assert(scene.events.item_count == scene.event_cells.size())
		print("Loaded and browsed ", scene.maps.size(), " installed GEO maps.")
	# A synthetic map makes marker and click behavior independent of game assets.
	var cells: Array = []
	for i in range(256): cells.append({"walls":[0,0,0,0], "doors":[0,0,0,0], "event":0})
	cells[35] = {"walls":[1,2,3,4], "doors":[0,1,2,3], "event":133}
	cells[36].event = 128
	var original: Array = scene.maps
	scene.maps = [{"archive":"GEO_TEST", "id":1, "bytes":1026, "cells":cells}]
	scene._select_map(0)
	assert(scene.event_cells == [35,36])
	scene._event_selected(0)
	assert(scene.selected == Vector2i(3,2))
	assert("Raw: 0x85" in scene.details.text)
	var click := InputEventMouseButton.new()
	click.button_index = MOUSE_BUTTON_LEFT
	click.pressed = true
	click.position = scene.grid_origin + Vector2(4.5,2.5) * scene.cell_size
	scene._gui_input(click)
	assert(scene.selected == Vector2i(4,2))
	scene.show_events.button_pressed = false
	scene.show_walls.button_pressed = false
	await process_frame
	scene.show_events.button_pressed = true
	scene.show_walls.button_pressed = true
	if not original.is_empty():
		scene.maps = original
		scene._select_map(0)
	if "--capture" in OS.get_cmdline_user_args():
		await process_frame
		await RenderingServer.frame_post_draw
		var path := ProjectSettings.globalize_path("res://../user-data/map-inspector.png")
		DirAccess.make_dir_recursive_absolute(path.get_base_dir())
		assert(root.get_texture().get_image().save_png(path) == OK)
		print("Screenshot: ", path)
	print("Map inspector tests passed.")
	quit()
