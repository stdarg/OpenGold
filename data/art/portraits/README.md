# Additional portrait heads

Ten new portrait heads inspired by the DOS Pool of Radiance portraits,
generated with the built-in image generation tool. Each PNG contains one adult
head on a black background, facing slightly right, without a body or costume.

| Species | Man | Woman |
| --- | --- | --- |
| Gnome | [Portrait](gnome-male.png) | [Portrait](gnome-female.png) |
| Orc | [Portrait](orc-male.png) | [Portrait](orc-female.png) |
| Goliath | [Portrait](goliath-male.png) | [Portrait](goliath-female.png) |
| Tiefling | [Portrait](tiefling-male.png) | [Portrait](tiefling-female.png) |
| Dragonborn | [Portrait](dragonborn-male.png) | [Portrait](dragonborn-female.png) |

The Goliath pair now uses slate-blue skin, russet-brown markings and amber
eyes. The color revision preserves their faces and framing; its edit prompts
are recorded alongside the original generation prompts.

These enlarged source PNGs are now used by the native character creator.
`build-rolf.cmd` copies them into `godot/bin/portraits/`, and the review launcher
imports those copies as Godot resources. At load time, native code fits each
head into the 88 x 40 panel with nearest-neighbor sampling and removes bottom
black padding so the neck reaches the original body panel. Their approved
colors are retained; they are not restricted to exactly sixteen EGA colors.

The portrait dropdown and head arrows include all ten new heads alongside the
original game's heads. Race/gender choices suggest a matching head until the
player makes a manual selection. See [Character creation](../../../docs/CHARACTER-CREATION.md).

The original portraits were used only as a local style/framing reference and
are not included here. [prompts.json](prompts.json) preserves the exact prompt
for each image. The PNGs preserve the generation outputs without resampling.
