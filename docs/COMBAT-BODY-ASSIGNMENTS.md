# Combat body assignments

`CBODY.DAX` has 32 base body IDs. Each ID also has a short/tall version and a
ready/action pose. The weapon and shield are drawn into each body; they are not
separate layers. The source palette identifies weapon and shield color regions,
but does not identify the kind of weapon. Review the complete figure visually.

The editable catalog is `data/art/combat-body-looks.tsv`. Each of its 32 rows
contains a base ID and one stable, complete look ID, separated by a tab. The
available look IDs and labels are in `data/art/combat-weapon-options.tsv`,
derived from the original arms and silver shop stock. The list includes the
46 sellable weapon types (excluding arrow and quarrel ammunition), seven named
silver variants, and Unarmed. Each offers a plain and shield look. The silver
shop's "Fine Composite Long Bow" uses original item type 45, which the general
item table calls Heavy Crossbow; the reviewer keeps the shop's displayed name.
The `unreviewed` value excludes a body from equipment selection. Some initial
assignments are visual judgments; inspect them in the review tool and correct
any mismatch. The game uses the same file and copies it into the package when
built. No original game art is stored in the catalog or repository.

On Windows, launch the separate reviewer with:

```powershell
$env:OPENGOLD_GAME_DIR = 'D:\path\to\POOLRAD\GAME\POOLRAD'
demos\review-combat-bodies.cmd
```

The launcher builds and checks the existing demo extension before opening the
review scene. The scene is also available directly at
`res://scenes/combat_body_review.tscn` within `demos/godot`.

The reviewer shows one body at a time, with short/tall and ready/action previews.
Previous/Next cycles through all 32 bodies. Type in the filter to narrow the
complete look list (for example, `mace` or `shield`). Selecting a row writes the
catalog immediately and shows the result or an error. Rebuild the game to include
an edit in its copied data file or package.

For combat, the game reads equipped inventory IDs and chooses the first body
with an exact weapon and shield match. If no exact match exists, it chooses a
body with the same weapon, regardless of shield. A silver weapon can use its
ordinary counterpart when no silver assignment exists. If no reviewed body depicts
that weapon, it keeps the character's saved combat body choice. The selected
body is used for both ready and action poses; the character's head, colors and
size remain theirs. Equipment is read when a combat scene is created.
The current SRD rules module permits only its supported weapon types to be
equipped; the reviewer lists the original shops' full stock for classification.
