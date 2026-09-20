extends SceneTree

func _initialize() -> void:
    call_deferred("check")

func check() -> void:
    root.size = Vector2i(1920, 1080)
    change_scene_to_file("res://scenes/combat_demo.tscn")
    for frame in range(8):
        await process_frame
    var combat := current_scene
    var canvas := combat.get_node("BattlefieldScroll/Canvas")
    var tile: float = canvas.custom_minimum_size.x / 12.0
    combat.get_node("Melee").emit_signal("pressed")
    var click := InputEventMouseButton.new()
    click.button_index = MOUSE_BUTTON_LEFT
    click.pressed = true
    click.position = canvas.get_global_transform_with_canvas() * Vector2(7.5 * tile, 4.5 * tile)
    root.push_input(click)
    for frame in range(300):
        if combat.get_node("Turn").text.contains("Merric Mistvale turn"):
            break
        var end_key := InputEventKey.new()
        end_key.keycode = KEY_ENTER
        end_key.pressed = true
        root.push_input(end_key)
        await process_frame
    if not combat.get_node("Turn").text.contains("Merric Mistvale turn"):
        push_error("Merric did not receive a turn to finish the wounded Kobold")
        quit(1)
        return
    combat.get_node("Melee").emit_signal("pressed")
    click.position = canvas.get_global_transform_with_canvas() * Vector2(7.5 * tile, 4.5 * tile)
    root.push_input(click)
    if not combat.get_node("Log").text.contains("Kobold 3 is defeated."):
        push_error("Expected a lethal attack in the shared combat scene")
        quit(1)
        return
    if not combat.get_node("DeathAudio").playing:
        push_error("Lethal attack did not play the original death sound")
        quit(1)
        return
    await RenderingServer.frame_post_draw
    var first := root.get_texture().get_image()
    var sample: Vector2 = canvas.get_global_transform_with_canvas() * Vector2(7 * tile + 12, 4 * tile + 12)
    if first.get_pixelv(Vector2i(sample)).r < 0.6:
        push_error("Death skull was not visible in the defeated Kobold's square")
        quit(1)
        return
    var first_path := ProjectSettings.globalize_path("res://../../../build/checks/combat-death-skull.png")
    first.save_png(first_path)
    await create_timer(1.2).timeout
    await RenderingServer.frame_post_draw
    var after := root.get_texture().get_image()
    sample = canvas.get_global_transform_with_canvas() * Vector2(7 * tile + 12, 4 * tile + 12)
    if after.get_pixelv(Vector2i(sample)).r > 0.3:
        push_error("The defeated Kobold's square did not clear after one second")
        quit(1)
        return
    var after_path := ProjectSettings.globalize_path("res://../../../build/checks/combat-death-cleared.png")
    after.save_png(after_path)
    print("Combat death presentation checks passed: ", first_path, " -> ", after_path)
    quit(0)
