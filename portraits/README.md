# Complete character portraits

The roster contains 36 independently composed portraits. The initial six were
approved for style, and the remaining 30 extend that direction. These files are
an artwork collection; game integration is a separate step.

Each PNG contains a complete chest-up character on a black background. Faces,
physiques, poses, and clothing are individually designed rather than assembled
from interchangeable heads and bodies. Each of the 12 classes has one Female,
one Male, and one Nonbinary portrait. Each of the nine races has four portraits.
All races include all three genders; Nonbinary briefs request androgynous presentation.
Real-world ancestry-inspired facial and hair characteristics vary across fantasy
races and are not game-rule categories or metadata fields.

`portraits.json` maps each PNG basename, including its extension, to exactly
`Class`, `Gender`, and `Race`, using the character creator's display names.
`prompts.json` records the original six prompts and `roster-prompts.json` records
the 30 expansion prompts, all used with the built-in image generator. The
original game's local contact sheet informed general palette, framing, and
readability of the approved six; the expansion uses those approved portraits
as a rendering-style reference. No extracted original assets are included here.

The four Orc portraits were subsequently revised to visibly green skin at the
user's request, retaining their individual ancestry-inspired facial features,
hair, expressions, costumes, and poses. `orc-green-revision-prompts.json` records
the built-in image-generation edit prompts. Their basenames and catalog metadata
remain the same. Earlier generation prompts document the original versions.

All 36 PNGs retain the generator's 1254 x 1254 full-resolution output. Pixel-art styling does
not guarantee a mathematically uniform low-resolution pixel grid. Local review
previews render each at 88 x 88 as well as enlarged sizes; these are visual
readability checks, not a claim of tested runtime integration or a separately
hand-tuned 88 x 88 asset set.

Local roster overview: `../user-data/portrait-review/roster-contact-sheet.png`.
Four `roster-detail-1.png` through `roster-detail-4.png` sheets in that directory
show 352 x 352 and native 88 x 88 previews, using nearest-neighbor sampling.
The initial six-image style review remains in `review-sheet.png`.

Coverage includes Barbarian, Bard, Cleric, Druid, Fighter, Monk, Paladin,
Ranger, Rogue, Sorcerer, Warlock, and Wizard; Dragonborn, Dwarf, Elf, Gnome,
Goliath, Halfling, Human, Orc, and Tiefling. This is a balanced selection, not
every possible class/race/gender combination. Metadata is descriptive and does
not impose restrictions on character creation.

Validation: all 36 PNGs decode, have unique file hashes, and correspond exactly
to catalog basenames and recorded prompts. Metadata fields and values match the
character creator; class, race, and gender coverage was checked. All portraits
were visually inspected individually and on the large/native-size review sheets.
