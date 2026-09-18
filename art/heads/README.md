# Additional combat heads

Ten original head designs for Tieflings, Goliaths, Gnomes, Dragonborn, and Orcs.
Each transparent source sheet contains two right-facing heads, ordered left to
right. These are combat head components, separate from the chest-up artwork in
`art/portraits`.

## Source artwork

| Species | Source sheet | Left design | Right design |
| --- | --- | --- | --- |
| Tiefling | [tiefling.png](sources/tiefling.png) | `tiefling-01`: upright horns | `tiefling-02`: swept horn |
| Goliath | [goliath.png](sources/goliath.png) | `goliath-01`: diagonal temple marking | `goliath-02`: brow and cheek markings |
| Gnome | [gnome.png](sources/gnome.png) | `gnome-01`: swept hair | `gnome-02`: short beard |
| Dragonborn | [dragonborn.png](sources/dragonborn.png) | `dragonborn-01`: swept crest | `dragonborn-02`: blunt horn and longer snout |
| Orc | [orc.png](sources/orc.png) | `orc-01`: topknot | `orc-02`: shaved scalp and marking |

These identifiers name artwork variants; they are not assigned game record IDs.
The PNGs preserve the generator's original resolution and alpha. They still need
cropping, reduction, and palette conversion into native combat components.
There are no native-size exports or runtime registrations in this directory yet.

## Approved native component requirements

- Export each design for both short and tall bodies, with ready and action
  variants. The existing composer overlays nonzero head pixels onto the top
  rows of a 24 × 24 body.
- Tall head canvases are **24 × 8 pixels**; short head canvases are **24 × 10
  pixels**. Preserve the neck position and transparent canvas padding.
- **Goliath heads must be the same native size as human heads.** The inspected
  human reference has a 5 × 6 visible footprint at `(10, 2)` on the tall canvas
  and a 7 × 5 footprint at `(9, 5)` on the short canvas. Coordinates are
  zero-based; both footprints reach their canvas's bottom edge. Do not enlarge
  the Goliath head asset because the character is taller.
- Horns, ears, and crests must fit within the component canvas. Check the tiny
  silhouettes after reduction; generated pixel-art styling does not guarantee
  a uniform native pixel grid.
- Preserve semantic combat indices: `0` is transparent, `8` is opaque black,
  `4` is the head's Color-1 region, and `12` is its Color-2 region. Hair, horns,
  or facial markings belong to the feature region; skin belongs to the skin
  region. Do not collapse black outlines into transparency or use body-region
  indices for head shading.
- Keep head placement stable between ready and action poses. The inspected
  original head pairs use identical pixels for these poses; the body supplies
  the action animation.

The native asset dimensions above are separate from the game's approved
[whole-sprite Goliath rendering](../../docs/TECH.md#goliath-combat-sprites-and-draw-order).
The composer and color-region mapping are in
[`character_art.cpp`](../../src/OpenGold.Core/src/character_art.cpp); the combat
image format is documented in [graphics-format.md](../../docs/graphics-format.md).

## Provenance and checks

Created on 2026-09-18 using the built-in image generator, one call per species
source sheet. [prompts.json](prompts.json) records the exact prompts. No original
game images were supplied as generation inputs or copied into this directory.

All five PNGs were visually inspected. Their PNG chunk checksums, decoded pixel
data lengths, RGBA format, and actual transparent and opaque pixels were
verified. Native-size silhouette, palette, and body-composition checks remain
part of the pending export step.
