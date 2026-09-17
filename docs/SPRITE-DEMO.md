# Combat sprite scale demo

The game includes an isolated sprite review scene using its native Godot
renderer, original asset decoders, character customization, theme, and screenshot
service. It does not create or modify a campaign.

## Launch

Build the [game application](../src/OpenGoldBox/README.md#build), then launch it
with `--sprite-demo`. From macOS bash at the repository root:

```bash
open -n mac-package/OpenGoldBox.app --args --sprite-demo
```

The usual language and original game folder settings apply. With a built
GDExtension, the source project can also run directly:

```bash
godot --path src/OpenGoldBox/godot -- --sprite-demo
```

## Controls and measurements

- The map shows short and normal players side by side, a Large Form comparison,
  and five nearby monsters. All eight figures alternate ready/action every second.
- Head, weapon, and both banks of six region colors update all three players.
  Regions absent from both player sizes are disabled. The numbered palette has
  color name tooltips; buttons support keyboard focus and activation.
- **−100%, −10%, +10%, +100%** change zoom by percentage points, from 10% to
  1000%. The initial zoom is 300%. At 100%, one source pixel is one rendered
  viewport pixel. Map and sprites scale together; controls retain their size.
- Scrollbars and the mouse wheel pan the map. Zoom retains the map location at
  the center where the scroll limits allow it.
- The readout lists source and displayed pixel dimensions for every sprite.
  Player measurements also include the visible, nontransparent bounds for the
  current pose. At 300%, a normal 24 × 24 sprite occupies 72 × 72 pixels;
  the Large Form comparison occupies 144 × 144 pixels.
- **Ctrl+S** saves a screenshot through the game's existing service.
  `python3 tools/screenshot.py` requests a capture externally. See
  [screenshot locations and options](../src/OpenGoldBox/README.md#screenshots).

## Goliath size

A normal Goliath is **Medium, about 7–8 feet tall**. Large Form changes its
size to Large. The species rule does not give the transformed character an exact
height. [Official Goliath species rules](https://www.dndbeyond.com/species/1751439-goliath).

Medium creatures control one 5-foot square; Large creatures control a 10-foot
square (2 × 2 grid squares). This space is not a measurement of bodily height.
The demo therefore offers **2× artwork as a visual comparison for that footprint**,
without claiming that Large Form is exactly 12 feet tall or adding an invented
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

CTest's `opengold_godot_sprite_demo` uses wholly synthetic archives and checks
customization, transparency, both zoom step sizes and limits, native proportions,
size readouts, the minimum window layout, and synchronized one-second poses.
To review those checks with installed art and capture both poses:

```bash
OPENGOLD_GAME_DIR=/path/to/POOLRAD OPENGOLD_LANG=en godot \
  --path src/OpenGoldBox/godot --script ../../../tests/combat_sprite_demo_tests.gd \
  -- --installed --capture
```
