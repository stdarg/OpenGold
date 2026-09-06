# OpenGold

A modern Godot-based reimplementation of SSI's Gold Box engine that reads the
original game data and assets while adding a cleaner UI, improved rendering,
and modern quality-of-life features.

## Native monster/NPC statistics

`opengold::por::CreatureCatalog` loads all original Pool of Radiance monster/NPC
records, their equipment, and special effects into reusable C++ values. It
provides stored HP, base AC/THAC0, both attack slots, saves, abilities, modifier
contributions and effect descriptions with unknown bytes retained.

`opengold::por::CreatureFactory` reuses that catalog to create independent monster
and NPC instances with mutable HP and consumable attack/movement turn budgets.
Instances share immutable stats and can outlive their factory. See the
[runtime instance API and example](docs/creature-catalog.md#creating-runtime-monsters-and-npcs)
for the boundary between action tracking and combat resolution.

Build and inspect a creature from the repository root:

```powershell
.\build.cmd
.\build\opengold_creatures.exe "D:\path\to\POOLRAD" "TROLL"
```

Omit `"TROLL"` to inspect all records. See [the C++ API and interpretation
limits](docs/creature-catalog.md) for integration, required assets and tests.

## Map inspector

The [ECL runtime and engine design](docs/SCRIPTS.md) documents the initial native
script interpreter and planned map-trigger/encounter integration. After building,
run `.\build\opengold_scripts.exe --demo` for a self-contained arithmetic, text,
and menu script. The tool also lists, inspects, and runs installed ECL records;
missing gameplay host capabilities and unbound engine variables produce explicit faults.

Run `build.cmd`, then `review-maps.cmd` to browse the original GEO maps as a
top-down grid with walls, doors, and numbered event markers. Click cells or event
locations to inspect their raw data. The demo uses the configured game directory
and the reusable native `MapCatalog` loader. Markers expose potential script
triggers; ECL execution and collision-checked movement are not implemented yet.
See [map loading, demo controls, and tests](docs/MAPS.md).

## Monster art review tool

Run `review-art.cmd` from the repository root, or:

```powershell
godot --path godot res://scenes/monster_art_review.tscn
```

The review tool is organized around unique art groups rather than monster records.
It groups identical decoded images (including dimensions, category and ordered
poses), retains all archive references, and shows combat pose pairs and encounter
distance variants together. The inspected installation has 214 unique groups
from 302 source groups.

- Filter combat icons, encounter sprites, character components, or combat
  effects/miscellaneous; use Left / Right to browse groups.
- Search all monster records, then Confirm selected or mark the association
  Incorrect. Confirm saves and advances to the next unresolved group in the
  current category, skipping confirmed and Unknown groups and wrapping at the end.
  Use Previous to return and confirm additional monsters for the same group.
- Customizable player character **body** and **head** are separate searchable
  assignment targets, suggested first for CBODY and CHEAD art respectively.
  Confirm these components without assigning them to a named monster. Saved
  associations use stable `PLAYER:BODY` and `PLAYER:HEAD` identifiers.
- Arrow, Hatchet, and Flask projectile are searchable assignment targets. Select
  one and use the assignment-name field to rename it. Flask is available for
  manual assignment; no unverified source record is automatically labeled flask.
- To rename a category filter, choose it and click Rename category. Display
  names are saved in `user://art-category-names.json`; stable category IDs and
  existing review decisions are retained.
- Click Add category and enter a unique name to create a category. Choose it
  directly in the searchable assignment list and Confirm selected, or choose it
  in the destination dropdown and click Move current group to category to
  categorize the displayed art. Categories and group membership persist across
  restarts, preserving existing assignments and review decisions.
- Suggested, confirmed, and rejected links remain distinct. Incorrect links
  leave identity unresolved unless another confirmed assignment exists.
- Unconfirmed art preselects the closest available name to a suggested identity
  in the assignment list. Rejected entries are excluded; selection alone does
  not confirm an association.
- The Script evidence tab shows ECL instructions, dialogue on possible encounter
  paths, NPC recruitment references, and bounded character/icon table lookups.
  Script-based candidates take priority over same-ID guesses. Rolf, Scribe,
  Tavern brawler, Priest of Bane / acolytes, City watch, and Trader / wagon seller
  become searchable when their dialogue is found. One sprite can have several
  identities. Archive banks are inferred; these are not gameplay confirmations.
- Next unreviewed skips confirmed and explicitly Unknown groups. Unknown / skip
  clears active confirmations for that group without erasing rejections.
- Edit a selected monster's name separately; saving a name never confirms art.
- Images use nearest-neighbor integer enlargement to a minimum 48 x 48, with
  no smoothing. Source pixels are unchanged.

Names remain in `user://monster-art-names.json`. Existing per-image Incorrect
marks are read from `user://monster-art-decisions.json` and conservatively applied
to the corresponding grouped association; those files are not rewritten by
migration. Explicit group review can supersede a legacy rejection.
New decisions are saved in `user://art-group-review.json`. A complete regenerated
inventory, source references, names, and effective links/statuses is saved in
`user://art-group-inventory.json`. Save files use temporary-file replacement.
The previous monster-art-associations.json is historical and is not updated by
this new view. Original DAX files are unchanged.

Script evidence is rebuilt from the selected game installation on startup and
saved separately in `user://art-script-evidence.json`. For an offline research
report, run `godot_console --headless --path godot --script ../tools/research_ecl_art.gd`.
This writes the decoded ECL index, evidence JSON, and a report for the saved
unresolved groups under ignored `user-data/`. See [NPC art findings](docs/npc-art-identification.md)
for the discoveries and remaining validation. Test with
`godot_console --headless --path godot --script ../tests/ecl_art_tests.gd`.

Current scope is CPIC, SPRIT, CHEAD, CBODY and COMSPR. Portraits, scene
illustrations, walls/terrain, and non-image assets still require their own
validated decoders; this tool does not claim to categorize those yet. Automatic
name suggestions use script evidence where available and fall back to provisional
matching-ID guesses; search allows manual assignment beyond those suggestions.
Save names before closing.

## Build setup

OpenGold uses CMake to build a portable C++20 native core, and the Godot project
lives under `godot/`.

Install the prerequisites listed in [docs/INSTALL.md](docs/INSTALL.md), then
configure and build from the repository root:

```powershell
cmake --preset default
cmake --build --preset default
ctest --test-dir build --output-on-failure
```

From a regular VS Code terminal, run the repository-local helper instead. It
initializes Visual Studio and uses its installed CMake directly:

```cmd
build.cmd
```

Open `godot/project.godot` in Godot 4.x for the presentation shell. The native
core is deliberately testable without launching Godot; GDExtension bindings
will be added at the presentation integration milestone.

The current scene compares nearest-neighbor, xBR level-2, and Maxim Stepin's HQx
(HQ4x) side by side at 4x. The dropdown defaults to **Combat sprites** (`CPIC*.DAX`,
`COMSPR.DAX`, `CHEAD.DAX`, and `CBODY.DAX`); **Encounter sprites** browses
`SPRIT*.DAX`. Combat starts at `CPIC1.DAX` record 2 when available.
Press Left / Right to browse every stored image across the selected category's
archives, wrapping at either end. Each category remembers its browsing position.
Archives and records are sorted numerically; the status line identifies the
archive, record, image, and overall position. All three panels stay synchronized.
Combat head/body components are shown individually, and combat poses stored in
separate records are browsable individually rather than automatically animated.
All views use the same background-composited image. The xBR shader retains
Hyllian's MIT license notice; the HQ4x shader and lookup table retain their
LGPL-2.1-or-later license and credits in [the HQx folder](godot/shaders/hqx/README.md).
Override the configured development path with the directory that
contains the DAX files:

```powershell
$env:OPENGOLD_GAME_DIR = 'C:\Games\POOLRAD'
```
