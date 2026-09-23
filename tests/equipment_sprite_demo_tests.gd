extends "combat_sprite_demo_tests.gd"

var demo: Control
var items: ItemList
var previews: Array = []

func click_at(position: Vector2, double_click := false) -> void:
    var motion := InputEventMouseMotion.new()
    motion.position = position
    root.push_input(motion, true)
    for down in [true, false]:
        var event := InputEventMouseButton.new()
        event.position = position
        event.button_index = MOUSE_BUTTON_LEFT
        event.pressed = down
        event.double_click = double_click and down
        root.push_input(event, true)
        await process_frame
    await settle()

func click_row(index: int) -> void:
    await click_at(items.global_position + items.get_item_rect(index).get_center())

func click_button(name: String) -> void:
    await click_at(demo.get_node(name).get_global_rect().get_center())

func check_pointer_equipment() -> void:
    await click_row(0)
    await click_button("Equip")
    require(items.get_item_metadata(0).equipped, "Pointer equips first weapon")
    await click_row(1)
    require(items.is_selected(1), "Pointer selects another weapon after equip")
    require(not demo.get_node("Equip").disabled, "Equip re-enables for another selected item")
    await click_at(items.global_position + items.get_item_rect(1).get_center(), true)
    require(items.get_item_metadata(1).equipped, "Pointer replaces first weapon with next weapon")
    require(not items.get_item_metadata(0).equipped, "Weapon replacement frees the previous weapon")
    await snapshot("weapon-swapped")
    await click_button("Unequip")
    require(not items.get_item_metadata(1).equipped, "Pointer unequips next weapon")

func choose(index: int) -> void:
    items.select(index)
    items.item_selected.emit(index)

func pixels() -> Array:
    return [demo.get_node("Ready").texture.get_image().get_data(), demo.get_node("Action").texture.get_image().get_data()]

func snapshot(name: String) -> void:
    if OS.get_cmdline_user_args().has("--capture"):
        await process_frame
        await RenderingServer.frame_post_draw
        var directory := ProjectSettings.globalize_path("res://../../build/equipment-demo-screenshots")
        DirAccess.make_dir_recursive_absolute(directory)
        require(root.get_texture().get_image().save_png(directory.path_join(name + ".png")) == OK, "Capture saved")

func run_checks() -> void:
    if not OS.get_cmdline_user_args().has("--original"):
        create_fixtures()
    root.size = Vector2i(1280, 800)
    demo = load("res://scenes/equipment_sprite_demo.tscn").instantiate()
    root.add_child(demo)
    await settle()
    items = demo.get_node("Items")
    require(items.item_count == 47, "Every catalog weapon loads: " + demo.get_node("Status").text)
    var shield := items.item_count
    require(not demo.get_node("Shield").button_pressed, "Separate shield toggle starts off")
    for index in range(items.item_count):
        require(items.get_item_metadata(index).type != 59, "Only weapons appear in the list")
    var unarmed := pixels()
    await check_pointer_equipment()
    await click_button("Shield")
    require(demo.get_node("Shield").button_pressed, "Pointer toggles shield on")
    await click_button("Shield")
    require(not demo.get_node("Shield").button_pressed, "Pointer toggles shield off")
    await snapshot("unarmed")
    # Independently read the shared classifications to verify the resolved body.
    var mappings := {}
    var deleted := []
    for line in FileAccess.get_file_as_string("res://../../data/art/combat-body-looks.tsv").split("\n"):
        var fields := line.strip_edges().split("\t")
        if fields.size() != 2 or fields[0].begins_with("#"):
            continue
        if fields[0] == "deleted":
            deleted.append(fields[1])
        else:
            for key in fields[1].split(","):
                if not mappings.has(key) or fields[0] == "24":
                    mappings[key] = int(fields[0])
    var changed := 0
    for index in range(shield):
        choose(index)
        var info: Dictionary = items.get_item_metadata(index)
        press(demo, "Equip")
        require(items.get_item_metadata(index).equipped, "Weapon equips")
        var weapon_pixels := pixels()
        require(demo.get_meta("body") == 24, "Every weapon retains the character body")
        previews.append([items.get_item_text(index).replace("[Equipped] ", ""), demo.get_node("Ready").texture, demo.get_node("Action").texture])
        if info.type in [1, 6]:
            await snapshot("battle-axe" if info.type == 1 else "bo-stick")
        if weapon_pixels != unarmed:
            changed += 1
        var key := "type_%d" % info.type
        var expected: int = mappings.get(key, 24) if not key in deleted else 24
        require(demo.get_meta("equipment_body") == expected, "Correct body for " + key)
        # Equipping another weapon replaces the occupied weapon slot.
        choose((index + 1) % shield)
        press(demo, "Equip")
        require(not items.get_item_metadata(index).equipped, "Previous weapon is released")
        require(items.get_item_metadata((index + 1) % shield).equipped, "Next weapon replaces previous weapon")
        choose(index)
        press(demo, "Equip")
        require(pixels() == weapon_pixels, "Swapping back restores both poses")
        press(demo, "Shield")
        require(demo.get_node("Shield").button_pressed == (info.hands == 1), "Shield enforces hand limit")
        if info.hands == 1:
            key += "_shield"
            expected = mappings.get(key, 24) if not key in deleted else 24
            require(demo.get_meta("equipment_body") == expected, "Correct shield body")
            if info.type == 36:
                await snapshot("long-sword-and-shield")
            # Swap with an occupied weapon slot AND shield. Rejection is atomic.
            var with_shield := pixels()
            var alternate := 1 if index == 0 else 0
            choose(alternate)
            press(demo, "Equip")
            require(demo.get_node("Shield").button_pressed, "Compatible shield survives weapon swap")
            require(items.get_item_metadata(alternate).equipped and not items.get_item_metadata(index).equipped, "Weapon swaps with shield equipped")
            choose(index)
            press(demo, "Equip")
            require(pixels() == with_shield, "Weapon and shield poses restored after swap")
            var two_handed := -1
            for candidate in range(shield):
                if items.get_item_metadata(candidate).hands == 2:
                    two_handed = candidate
                    break
            require(two_handed >= 0, "Two-handed test weapon exists")
            choose(two_handed)
            press(demo, "Equip")
            require(items.get_item_metadata(index).equipped and demo.get_node("Shield").button_pressed, "Rejected swap retains previous weapon and shield")
            require(not items.get_item_metadata(two_handed).equipped and pixels() == with_shield, "Rejected swap preserves both previews")
            press(demo, "Shield")
            require(pixels() == weapon_pixels, "Removing shield restores both poses")
        else:
            require(pixels() == weapon_pixels, "Rejected shield preserves both poses")
        choose(index)
        press(demo, "Unequip")
        require(pixels() == unarmed, "Removing weapon restores both unarmed poses")
        # Reverse order: shield first must also block two-handed weapons.
        press(demo, "Shield")
        var shield_pixels := pixels()
        choose(index)
        press(demo, "Equip")
        require(items.get_item_metadata(index).equipped == (info.hands == 1), "Weapon respects an equipped shield")
        if info.hands == 1:
            press(demo, "Unequip")
        else:
            require(pixels() == shield_pixels, "Rejected weapon preserves shield artwork")
        press(demo, "Shield")
    require(changed > 30, "Many weapon previews visibly differ from unarmed")
    choose(0)
    require(not demo.get_node("Equip").disabled and demo.get_node("Unequip").disabled, "Button availability follows selection")
    if OS.get_cmdline_user_args().has("--capture"):
        demo.hide()
        root.size = Vector2i(1440, 900)
        var gallery := Control.new()
        root.add_child(gallery)
        for index in range(previews.size()):
            var origin := Vector2((index % 8) * 180, (index / 8) * 145)
            var label := Label.new()
            label.text = previews[index][0].split("  (")[0]
            label.position = origin + Vector2(4, 4)
            label.add_theme_font_size_override("font_size", 13)
            gallery.add_child(label)
            for pose in range(2):
                var sprite := TextureRect.new()
                sprite.texture = previews[index][pose + 1]
                sprite.texture_filter = CanvasItem.TEXTURE_FILTER_NEAREST
                sprite.expand_mode = TextureRect.EXPAND_IGNORE_SIZE
                sprite.stretch_mode = TextureRect.STRETCH_KEEP_ASPECT_CENTERED
                sprite.position = origin + Vector2(4 + pose * 86, 26)
                sprite.size = Vector2(84, 108)
                gallery.add_child(sprite)
        await settle()
        await snapshot("all-weapons")
        gallery.queue_free()
    demo.queue_free()
    await settle()
    cleanup()
    print("Equipment sprite demo checks passed: pointer weapon swaps, all 47 weapons, shield, atomic rejection, both poses and unarmed")
    quit(0)
