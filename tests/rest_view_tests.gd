extends SceneTree
func _initialize() -> void:
    call_deferred("start")
func start() -> void:
    change_scene_to_file("res://scenes/rolf_tour.tscn")
