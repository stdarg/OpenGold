extends Control

const CATALOG := "res://../../data/art/combat-body-looks.tsv"
const OPTIONS := "res://../../data/art/combat-weapon-options.tsv"

var catalog_path := CATALOG
var options_path := OPTIONS
var loaded := false
var body_id := 0
var assignments: Array = []
var looks: Array = []
var body_data: PackedByteArray
var head_data: PackedByteArray
var loader := DaxSpriteLoader.new()
var number: Label
var assignment: Label
var status: Label
var filter_box: LineEdit
var list: VBoxContainer
var summary: RichTextLabel
var previews: Array[TextureRect] = []

func _ready() -> void:
    _build_ui()
    if not _load_catalog(): return
    var folder := OS.get_environment("OPENGOLD_GAME_DIR")
    if folder.is_empty(): folder = str(ProjectSettings.get_setting("opengold/game_directory", ""))
    if folder.is_empty():
        _fail("Set OPENGOLD_GAME_DIR to the original game folder.")
        return
    body_data = FileAccess.get_file_as_bytes(folder.path_join("CBODY.DAX"))
    head_data = FileAccess.get_file_as_bytes(folder.path_join("CHEAD.DAX"))
    if body_data.is_empty() or head_data.is_empty():
        _fail("Cannot read CBODY.DAX or CHEAD.DAX in " + folder)
        return
    _refresh()
    status.text = "Changes save immediately to " + ProjectSettings.globalize_path(catalog_path)

func _build_ui() -> void:
    var background := ColorRect.new()
    background.color = Color("17232d")
    background.set_anchors_and_offsets_preset(Control.PRESET_FULL_RECT)
    add_child(background)
    var panel := MarginContainer.new()
    panel.set_anchors_and_offsets_preset(Control.PRESET_FULL_RECT)
    panel.add_theme_constant_override("margin_left", 24)
    panel.add_theme_constant_override("margin_right", 24)
    panel.add_theme_constant_override("margin_top", 18)
    panel.add_theme_constant_override("margin_bottom", 18)
    add_child(panel)
    var column := VBoxContainer.new()
    column.add_theme_constant_override("separation", 12)
    panel.add_child(column)
    var title := Label.new()
    title.text = "Combat body assignments"
    title.add_theme_font_size_override("font_size", 28)
    column.add_child(title)
    var help := Label.new()
    help.text = "One original CBODY.DAX body at a time. Four previews show short/tall and ready/action poses. Check every combination this artwork can represent. Each change saves immediately."
    help.autowrap_mode = TextServer.AUTOWRAP_WORD_SMART
    column.add_child(help)
    var nav := HBoxContainer.new()
    column.add_child(nav)
    var previous := Button.new()
    previous.text = "◀ Previous body"
    previous.pressed.connect(func(): _step(-1))
    nav.add_child(previous)
    number = Label.new()
    number.custom_minimum_size.x = 180
    number.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
    nav.add_child(number)
    var next := Button.new()
    next.text = "Next body ▶"
    next.pressed.connect(func(): _step(1))
    nav.add_child(next)
    assignment = Label.new()
    column.add_child(assignment)
    var content := HBoxContainer.new()
    content.size_flags_vertical = Control.SIZE_EXPAND_FILL
    content.add_theme_constant_override("separation", 24)
    column.add_child(content)
    var preview_grid := GridContainer.new()
    preview_grid.columns = 2
    preview_grid.size_flags_horizontal = Control.SIZE_EXPAND_FILL
    content.add_child(preview_grid)
    for label_text in ["Short · Ready", "Tall · Ready", "Short · Action", "Tall · Action"]:
        var cell := VBoxContainer.new()
        cell.size_flags_horizontal = Control.SIZE_EXPAND_FILL
        preview_grid.add_child(cell)
        var label := Label.new()
        label.text = label_text
        cell.add_child(label)
        var texture := TextureRect.new()
        texture.custom_minimum_size = Vector2(180, 180)
        texture.size_flags_horizontal = Control.SIZE_EXPAND_FILL
        texture.size_flags_vertical = Control.SIZE_EXPAND_FILL
        texture.expand_mode = TextureRect.EXPAND_IGNORE_SIZE
        texture.stretch_mode = TextureRect.STRETCH_KEEP_ASPECT_CENTERED
        texture.texture_filter = CanvasItem.TEXTURE_FILTER_NEAREST
        cell.add_child(texture)
        previews.append(texture)
    var selection := VBoxContainer.new()
    selection.custom_minimum_size.x = 410
    selection.size_flags_vertical = Control.SIZE_EXPAND_FILL
    content.add_child(selection)
    var list_label := Label.new()
    list_label.text = "Equipment combinations"
    selection.add_child(list_label)
    filter_box = LineEdit.new()
    filter_box.placeholder_text = "Filter looks (e.g. mace or shield)"
    filter_box.text_changed.connect(func(_value): _fill_list())
    selection.add_child(filter_box)
    var scroll := ScrollContainer.new()
    scroll.size_flags_vertical = Control.SIZE_EXPAND_FILL
    selection.add_child(scroll)
    list = VBoxContainer.new()
    list.size_flags_horizontal = Control.SIZE_EXPAND_FILL
    scroll.add_child(list)
    var summary_label := Label.new()
    summary_label.text = "All assignments for this body (including filtered out)"
    selection.add_child(summary_label)
    summary = RichTextLabel.new()
    summary.custom_minimum_size.y = 110
    selection.add_child(summary)
    status = Label.new()
    status.autowrap_mode = TextServer.AUTOWRAP_WORD_SMART
    column.add_child(status)

func _load_catalog() -> bool:
    if not FileAccess.file_exists(options_path):
        _fail("Missing look options: " + ProjectSettings.globalize_path(options_path))
        return false
    var ids := {"unreviewed": true}
    for line in FileAccess.get_file_as_string(options_path).split("\n"):
        if line.is_empty() or line.begins_with("#"): continue
        var fields := line.strip_edges().split("\t")
        if fields.size() != 4 or not fields[2].is_valid_int() or fields[0].is_empty() or fields[1].is_empty() or fields[3] != "ordinary":
            _fail("Invalid look option: " + line)
            return false
        var id: String = fields[0]
        if ids.has(id) or ids.has(id + "_shield"):
            _fail("Duplicate look option: " + id)
            return false
        ids[id] = true
        ids[id + "_shield"] = true
        looks.append([id, fields[1]])
        looks.append([id + "_shield", fields[1] + " & Shield"])
    if not FileAccess.file_exists(catalog_path):
        _fail("Missing catalog: " + ProjectSettings.globalize_path(catalog_path))
        return false
    assignments.resize(32)
    var seen := {}
    for line in FileAccess.get_file_as_string(catalog_path).split("\n"):
        if line.is_empty() or line.begins_with("#"): continue
        var fields := line.strip_edges().split("\t")
        if fields.size() != 2 or not fields[0].is_valid_int():
            _fail("Invalid catalog row: " + line)
            return false
        var id := int(fields[0])
        if id < 0 or id >= 32 or seen.has(id):
            _fail("Invalid catalog assignment: " + line)
            return false
        seen[id] = true
        var values: Array[String] = []
        if fields[1] != "unreviewed":
            for value in fields[1].split(","):
                var key: String = value
                if key.begins_with("silver_"): key = "type_" + key.substr(7)
                if not _known(key):
                    _fail("Unknown assignment: " + key)
                    return false
                if not values.has(key): values.append(key)
        values.sort()
        assignments[id] = values
    if seen.size() != 32:
        _fail("Catalog must contain all 32 body IDs.")
        return false
    loaded = true
    return true

func _known(key: String) -> bool:
    for look in looks:
        if look[0] == key: return true
    return false

func _name_for(key: String) -> String:
    for look in looks:
        if look[0] == key: return look[1]
    return key

func _step(amount: int) -> void:
    if not loaded: return
    body_id = posmod(body_id + amount, 32)
    _refresh()

func _refresh() -> void:
    number.text = "Body %d / 31" % body_id
    _show_assignments()
    for variant in range(4):
        var size_offset: int = 0 if variant % 2 == 0 else 64
        var pose_offset: int = 0 if variant < 2 else 128
        var body := loader._extract_record(body_data, body_id + size_offset + pose_offset)
        var head := loader._extract_record(head_data, size_offset + pose_offset)
        if body.size() < 305 or head.size() < 17:
            _fail("Cannot decode original body or head: " + loader.error_message)
            return
        var image := _compose(head, body)
        previews[variant].texture = ImageTexture.create_from_image(image)
    _fill_list()

func _compose(head: PackedByteArray, body: PackedByteArray) -> Image:
    var pixels := PackedByteArray()
    pixels.resize(576)
    var head_count: int = int(head[0]) * 24
    for p in range(576):
        var source: int = body[17 + (p >> 1)]
        var index: int = source >> 4 if p % 2 == 0 else source & 15
        if p < head_count:
            source = head[17 + (p >> 1)]
            var overlay: int = source >> 4 if p % 2 == 0 else source & 15
            if overlay != 0: index = overlay
        pixels[p] = index
    var image := Image.create(24, 24, false, Image.FORMAT_RGBA8)
    for p in range(576):
        var index: int = pixels[p]
        var color: Color = DaxSpriteLoader.EGA_PALETTE[index]
        if index == 8: color = Color.BLACK
        if index == 13: color = Color("ff55ff")
        if index == 0: color.a = 0
        image.set_pixel(p % 24, p / 24, color)
    return image

func _show_assignments() -> void:
    var names := PackedStringArray()
    for key in assignments[body_id]: names.append(_name_for(key))
    assignment.text = "Unreviewed - no assignments" if names.is_empty() else "%d assigned combinations" % names.size()
    summary.text = "Unreviewed" if names.is_empty() else "\n".join(names)

func _fill_list() -> void:
    for child in list.get_children():
        list.remove_child(child)
        child.queue_free()
    if not loaded: return
    var query := filter_box.text.strip_edges().to_lower()
    for i in range(looks.size()):
        if not query.is_empty() and not str(looks[i][1]).to_lower().contains(query): continue
        var checkbox := CheckBox.new()
        checkbox.text = looks[i][1]
        checkbox.button_pressed = assignments[body_id].has(looks[i][0])
        checkbox.toggled.connect(func(checked): _toggle(looks[i][0], checked))
        list.add_child(checkbox)

func _toggle(key: String, checked: bool) -> void:
    if not loaded or not _known(key): return
    var next: Array = assignments.duplicate(true)
    if checked and not next[body_id].has(key): next[body_id].append(key)
    if not checked: next[body_id].erase(key)
    next[body_id].sort()
    if not _save(next):
        _fill_list()
        return
    assignments = next
    _show_assignments()

func _save(next: Array) -> bool:
    var content := "# v2: CBODY.DAX base ID, then comma-separated combination IDs; unreviewed means no associations.\n"
    for id in range(32):
        content += "%d\t%s\n" % [id, "unreviewed" if next[id].is_empty() else ",".join(next[id])]
    var temporary := catalog_path + ".tmp"
    var file := FileAccess.open(temporary, FileAccess.WRITE)
    if file == null:
        _fail("Save failed: " + error_string(FileAccess.get_open_error()))
        return false
    file.store_string(content)
    file.flush()
    var write_error := file.get_error()
    file.close()
    if write_error != OK:
        _fail("Save failed: " + error_string(write_error))
        return false
    var error := DirAccess.rename_absolute(ProjectSettings.globalize_path(temporary), ProjectSettings.globalize_path(catalog_path))
    if error != OK:
        _fail("Save failed: " + error_string(error))
        return false
    status.text = "Saved " + ProjectSettings.globalize_path(catalog_path) + ". Rebuild the game to package this edit."
    return true

func _fail(message: String) -> void:
    status.text = message
    push_error(message)
