# Character creation

The standalone C++20/Godot demo creates one level-one character and shows the
character sheet. It uses SRD 5.2.1 and the original game's portrait/combat art.

Run from Windows CMD at the repository root:

```cmd
build-rolf.cmd
review-character.cmd
```

The shared extension uses the existing [build prerequisites](ROLF.md). The
launcher opens `godot/scenes/character_creation.tscn`. Art loads from the
`OPENGOLD_GAME_DIR` environment variable, falling back to
`opengold/game_directory` in `godot/project.godot`.

## Creation flow

1. Select race (the nine species included in SRD 5.2.1).
2. Select gender. Gender does not alter stats or restrict other choices.
3. Select one of the twelve SRD classes.
4. Select alignment.
5. Roll attributes. All four dice appear, with the discarded lowest die marked.
   Click two attribute buttons to swap their rolled results. **Reroll all six**
   replaces the complete set and resets assignments; attempts are unlimited.
   Choose one of the four SRD backgrounds here, then allocate its attribute
   bonuses (+2/+1 to different allowed abilities, or +1 to all three).
6. Review maximum starting HP: maximum class Hit Die + Constitution modifier,
   with +1 for Dwarven Toughness when applicable.
7. Enter a name, up to 40 characters.
8. Browse portrait heads and bodies with the previous/next buttons.
9. Customize combat head and weapon/body parts, tall/short art, and all twelve
   region colors. Select a region's Color-1 or Color-2 button, then a palette
   swatch. Enlarged ready and action previews update immediately, recoloring
   only that part. Controls show **Not present** when the selected parts omit
   that region in both poses. Its saved colors return when the part is present.
10. Show the character sheet: identity, level, background, six scores and
    modifiers, base scores/bonuses, Hit Dice, the HP calculation, and inventory
    (initially empty).

**Back** preserves selections and allows earlier edits. Derived scores and HP
update from the current choices. **Start over** clears the single character.

Per the confirmed scope, this demo has no pool, party management, persistence,
or campaign integration. The later party limit remains six PCs plus two NPC
positions. Class feature choices, skills, equipment, spells, and species
lineage choices are outside this first requested flow; the sheet presents the
implemented creation fields, not a complete combat-ready character. The Rolf
fighter and combat fixtures remain separate.

## Native boundaries and rules

- `OpenGold.Rules/character_rules.h` defines the optional `CharacterRules`
  capability, draft, retained dice, choices, adjustments and sheet values.
- `OpenGold.Rules.Srd5` supplies the SRD choices and arithmetic. It has no Godot,
  original-asset, or campaign dependency. Dice use seeded SplitMix64 with
  rejection sampling; invalid assignments and selections are rejected.
- `OpenGold.Core/character_creator.h` owns the injected rules module and the
  single character's sequence and appearance. A completed sheet must return to
  editing before mutation. No rolled result can be assigned twice.
  `create_character()` exports a finished `Character` only after the last step.
- `OpenGold.Core/character.h` is the reusable, value-owned character model.
  It retains all creation choices, original dice, assignments and bonuses;
  the evaluated sheet with rules identity and HP; portrait/part references;
  both combat color banks; and an `Inventory`. It has no dependency on Godot
  or the lifetime of the creator, rules module, or loaded artwork. The final
  demo sheet reads this object. Appearance changes are validated, and copying
  a character produces independent appearance and inventory data.
- `OpenGold.Core/inventory.h` owns item stacks with stable inventory-local IDs,
  content definition keys, display names and quantities. It supports addition,
  lookup and partial/full removal, rejecting invalid operations without losing
  items. Separate additions remain separate stacks. Rules or content adapters
  resolve the definition keys; inventory storage imposes no edition-specific
  equipment rules or legacy item limit. Starting gear selection, equipping,
  shop integration and saving remain outside this standalone demo.
- `CharacterCreationView` is native GDExtension presentation and input, backed
  by a Godot scene. Godot owns scene nodes; resource references use Godot RAII.

The rolling and HP rules come from
[SRD 5.2.1, pp. 21–22](https://media.dndbeyond.com/compendium-images/srd/5.2/SRD_CC_v5.2.1.pdf#page=21),
backgrounds from p. 83, and Dwarven Toughness from p. 84. Unlimited full-set
rerolls are the user-approved customization. Attribution is in
[NOTICE.md](../data/rules/srd-5.2.1/NOTICE.md).

## Original character art

`CharacterArt` in `OpenGold.Core` reads the installed game's DAX files, retaining
archive IDs and indexed combat pixels. No extracted artwork is distributed.

- Portrait heads use `HEAD1..8.DAX`; bodies use `BODY1..8.DAX`. The loader merges
  identical copies of an ID and rejects conflicting copies. It exposes the
  available original parts, without claiming a race/class-specific selection
  list. A head is 88 x 40 pixels, followed by an 88 x 48 body.
- Combat heads come from `CHEAD.DAX`, base IDs 0..13. Body/weapon components come
  from `CBODY.DAX`, base IDs 0..31. Add 64 for tall components and 128 for the
  action pose. Both head and body use the same size/pose bank. The head overlays
  the body's upper rows to produce a 24 x 24 icon.
- Source pixel 0 is transparent; source pixel 8 is an opaque black outline.
  Recoloring operates on source indices, so choosing black never makes a region
  transparent. The two poses share the same customization settings.

| Region | Color-1 source index | Color-2 source index |
| --- | --- | --- |
| Weapon | 7 | 15 |
| Body | 1 | 9 |
| Hair / face | 4 (hair) | 12 (face) |
| Shield | 6 | 14 |
| Arms | 2 | 10 |
| Legs | 3 | 11 |

Indices 5 and 13 belong to the cap and remain fixed. This corrects the original
demo's misassignment of the weapon, shield and cap masks. In the installed
`CBODY.DAX`, base ID 0 is unarmed and has no weapon or shield indices; base ID 1
uses 7/15 for its bow and has no 6/14 shield indices. Base ID 4 includes both
weapon and shield regions. All six regions retain two independently selectable
colors from the 16-color EGA palette. Controls use visible pixels after head
composition to determine whether a region is available in either pose.
Portrait colors are separate from combat-icon colors.

The assembly and mapping were checked against the installed DAX records. The
original manual's Icon menu describes independent head/weapon selection, two
poses and two sizes. Stephen S. Lee's
[binary-format research](https://gamefaqs.gamespot.com/c64/578753-pool-of-radiance/faqs/73869)
also identifies the component archives and size bank. Its portrait-menu indices
are not used as an assumed list of available archive IDs.

## Verification

`build.cmd` and `build-rolf.cmd` include `opengold_character_tests`. Synthetic
tests cover deterministic rolls, retained dice, swaps, all class HP values,
background bonuses, invalid selections, name validation, navigation, restart,
truncation, all twelve color controls, head composition, fixed pixels,
transparency, character data ownership and inventory operations. Set
`OPENGOLD_GAME_DIR` to the directory containing
the DAX files to additionally check every original head/body combination in
both sizes and both poses, plus exact recolor masks and absent regions in the
original art:

```bat
set "OPENGOLD_GAME_DIR=D:\path\to\POOLRAD"
build.cmd
```

Run the Godot scene's automated creation check:

```cmd
godot --headless --path godot res://scenes/character_creation.tscn -- --character-check
```

This exercises choices, rolling, swaps, background bonuses, HP, name entry,
portrait/parts selection, all twelve colors, sheet review and restart. Each
palette click checks the preview texture pixels against the composed icon and
counts changed pixels in each pose; it also checks that the portrait is intact.
The completed sheet is checked against the reusable character and inventory. Buttons
and list selections use injected viewport mouse input; name entry uses keyboard
events. Background controls also exercise their native selection signals.
For local screenshots of attributes, appearance and the sheet, omit
`--headless` and append `--capture`. Captures go to ignored `user-data/` files.
