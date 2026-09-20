extends Control

const CATALOG := "res://../../data/art/combat-body-looks.tsv"
const OPTIONS := "res://../../data/art/combat-weapon-options.tsv"

var body_id := 0
var assignments: Array[String] = []
var looks := [["unreviewed", "Unreviewed"]]
var filtered: Array[int] = []
var body_data: PackedByteArray
var head_data: PackedByteArray
var loader := DaxSpriteLoader.new()
var number: Label
var assignment: Label
var status: Label
var filter_box: LineEdit
var list: ItemList
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
    status.text = "Changes save immediately to " + ProjectSettings.globalize_path(CATALOG)

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
    help.text = "One original CBODY.DAX body at a time. Four previews show short/tall and ready/action poses. Select a complete look to save it."
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
        texture.custom_minimum_size = Vector2(220, 220)
        texture.size_flags_horizontal = Control.SIZE_EXPAND_FILL
        texture.size_flags_vertical = Control.SIZE_EXPAND_FILL
        texture.expand_mode = TextureRect.EXPAND_IGNORE_SIZE
        texture.stretch_mode = TextureRect.STRETCH_KEEP_ASPECT_CENTERED
        texture.texture_filter = CanvasItem.TEXTURE_FILTER_NEAREST
        cell.add_child(texture)
        previews.append(texture)
    var selection := VBoxContainer.new()
    selection.custom_minimum_size.x = 290
    selection.size_flags_vertical = Control.SIZE_EXPAND_FILL
    content.add_child(selection)
    var list_label := Label.new()
    list_label.text = "Assign complete look"
    selection.add_child(list_label)
    filter_box = LineEdit.new()
    filter_box.placeholder_text = "Filter looks (e.g. mace or shield)"
    filter_box.text_changed.connect(func(_value): _fill_list())
    selection.add_child(filter_box)
    list = ItemList.new()
    list.select_mode = ItemList.SELECT_SINGLE
    list.size_flags_vertical = Control.SIZE_EXPAND_FILL
    list.item_selected.connect(_choose)
    selection.add_child(list)
    status = Label.new()
    status.autowrap_mode = TextServer.AUTOWRAP_WORD_SMART
    column.add_child(status)

func _load_catalog() -> bool:
    if not FileAccess.file_exists(OPTIONS):
        _fail("Missing look options: " + ProjectSettings.globalize_path(OPTIONS))
        return false
    var ids := {"unreviewed": true}
    for line in FileAccess.get_file_as_string(OPTIONS).split("\n"):
        if line.is_empty() or line.begins_with("#"): continue
        var fields := line.strip_edges().split("\t")
        if fields.size() != 4 or not fields[2].is_valid_int() or fields[0].is_empty() or fields[1].is_empty() or (fields[3] != "ordinary" and fields[3] != "silver"):
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
    if not FileAccess.file_exists(CATALOG):
        _fail("Missing catalog: " + ProjectSettings.globalize_path(CATALOG))
        return false
    assignments.resize(32)
    var seen := {}
    for line in FileAccess.get_file_as_string(CATALOG).split("\n"):
        if line.is_empty() or line.begins_with("#"): continue
        var fields := line.strip_edges().split("\t")
        if fields.size() != 2 or not fields[0].is_valid_int():
            _fail("Invalid catalog row: " + line)
            return false
        var id := int(fields[0])
        if id < 0 or id >= 32 or seen.has(id) or not _known(fields[1]):
            _fail("Invalid catalog assignment: " + line)
            return false
        seen[id] = true
        assignments[id] = fields[1]
    if seen.size() != 32:
        _fail("Catalog must contain all 32 body IDs.")
        return false
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
    body_id = posmod(body_id + amount, 32)
    _refresh()

func _refresh() -> void:
    number.text = "Body %d / 31" % body_id
    assignment.text = "Current assignment: " + _name_for(assignments[body_id])
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

func _fill_list() -> void:
    list.clear()
    filtered.clear()
    var query := filter_box.text.strip_edges().to_lower()
    for i in range(looks.size()):
        if not query.is_empty() and not str(looks[i][1]).to_lower().contains(query): continue
        filtered.append(i)
        list.add_item(looks[i][1])
        if looks[i][0] == assignments[body_id]: list.select(list.item_count - 1)

func _choose(row: int) -> void:
    var next: Array[String] = assignments.duplicate()
    next[body_id] = looks[filtered[row]][0]
    var content := "# CBODY.DAX base ID, then complete look ID. Edit with demos/godot/scenes/combat_body_review.tscn.\n"
    for id in range(32): content += "%d\t%s\n" % [id, next[id]]
    var temporary := CATALOG + ".tmp"
    var file := FileAccess.open(temporary, FileAccess.WRITE)
    if file == null:
        _fail("Save failed: " + error_string(FileAccess.get_open_error()))
        return
    file.store_string(content)
    file.flush()
    file = null
    var error := DirAccess.rename_absolute(ProjectSettings.globalize_path(temporary), ProjectSettings.globalize_path(CATALOG))
    if error != OK:
        _fail("Save failed: " + error_string(error))
        return
    assignments = next
    assignment.text = "Current assignment: " + _name_for(assignments[body_id])
    status.text = "Saved " + ProjectSettings.globalize_path(CATALOG) + ". Rebuild the game to package this edit."

func _fail(message: String) -> void:
    status.text = message
    push_error(message)
