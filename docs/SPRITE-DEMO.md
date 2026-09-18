# Combat sprite scale demo

This standalone review scene belongs to the existing `demos/godot` project and
its C++ `opengold_godot` extension. It uses the original asset decoders and
character customization without launching or extending the game application.

## Launch

Build the demo extension, then launch the demo. From macOS bash at the repository
root (replace the asset path with your original game installation):

```bash
cmake -S . -B build/sprite-demo -DCMAKE_BUILD_TYPE=RelWithDebInfo \
  -DOPENGOLD_BUILD_GODOT=ON -DOPENGOLD_BUILD_GAME=OFF
cmake --build build/sprite-demo --target opengold_godot -j 8
OPENGOLD_GAME_DIR=/path/to/POOLRAD bash demos/review-sprites.sh
```

The launcher locates Godot on PATH or in its standard macOS application folder;
`GODOT_BIN` can select another executable. On Windows, run
`demos\review-sprites.cmd`; it builds and checks the demo extension before
opening the scene so an older DLL cannot hide `CombatSpriteDemo`.

The demo reads `OPENGOLD_GAME_DIR`, falling back to the demo project's
`opengold/game_directory` setting. It has no game startup or settings flow.
You can also launch the scene directly:

```bash
godot --path demos/godot --resolution 1920x1080 res://scenes/combat_sprite_demo.tscn
```

## Controls and measurements

- The map shows short and normal players side by side, a Large Form comparison,
  and five nearby monsters. All eight figures alternate ready/action every second.
- Head, weapon, and both banks of six region colors update all three players.
  Regions absent from both player sizes are disabled. The numbered palette has
  color name tooltips; buttons support keyboard focus and activation.
- **−100%, −10%, +10%, +100%** change zoom by percentage points, from 10% to
  1000%. The initial zoom is 250%. At 100%, one source pixel is one rendered
  viewport pixel. Map and sprites scale together; controls retain their size.
- Scrollbars and the mouse wheel pan the map. Zoom retains the map location at
  the center where the scroll limits allow it.
- The readout lists source and displayed pixel dimensions for every sprite.
  Player measurements also include the visible, nontransparent bounds for the
  current pose. At 250%, a normal 24 × 24 sprite occupies 60 × 60 pixels;
  the synthetic Large Form's ready art is 60 × 80 pixels within a 60 × 120
  pixel, one-square-wide and two-square-tall footprint, aligned at its bottom.
- **Ctrl+S** saves a timestamped PNG in `user://sprite-demo-screenshots`.
  Set `OPENGOLD_SCREENSHOT_DIR` to an absolute folder to choose another location.
  The demo exposes `request_capture()` and `capture_completed(path, error)` for
  automated visual checks, without loading the game's screenshot autoload.

## Goliath size

A normal Goliath is **Medium, about 7–8 feet tall**. Large Form changes its
size to Large. The species rule does not give the transformed character an exact
height. [Official Goliath species rules](https://www.dndbeyond.com/species/1751439-goliath).

Medium creatures control one 5-foot square; Large creatures control a 10-foot
square (2 × 2 grid squares). This space is not a measurement of bodily height.
The demo draws a one-square-wide, two-square-tall footprint. It scales the
ready pose uniformly to fit the width and at most 75% of the height, then
centers its visible art horizontally and anchors it at the bottom. The action pose uses
the same scale. This does not claim that Large Form is exactly
12 feet tall or adding an invented
height rule to combat. See the creature size and Goliath sections of
[SRD 5.2.1](https://media.dndbeyond.com/compendium-images/srd/5.2/SRD_CC_v5.2.1.pdf).

## Assets and checks

Players reuse the original short/tall `CHEAD.DAX` and `CBODY.DAX` banks and the
same indexed recoloring as character creation. Both banks have a 24 × 24 canvas;
the short figure's visible pixels occupy less of it. Large Form scales the tall
artwork uniformly. Monster dimensions retain their original proportions.

The authored room uses the normal dungeon battlefield generator and locally
decoded `DUNGCOM.DAX` tiles. The five deliberate `CPIC2.DAX` art-review samples
are records 0, 2, 4, 26, and 31 (Kobold, Goblin, Orc, Basilisk, Troll), with 128
added for action poses. These selections do not change gameplay monster bindings.
Original artwork and screenshots are not included in the repository.

After building the demo extension, run:

```bash
ctest --test-dir build/sprite-demo -L demo --output-on-failure
```

The `opengold_godot_sprite_demo` check uses wholly synthetic archives. It checks
customization, transparency, both zoom step sizes and limits, native proportions,
size readouts, the minimum window layout, and synchronized one-second poses.
To review those checks with installed art and capture both poses:

```bash
OPENGOLD_GAME_DIR=/path/to/POOLRAD godot \
  --path demos/godot --script ../../tests/combat_sprite_demo_tests.gd \
  -- --installed --capture
```
