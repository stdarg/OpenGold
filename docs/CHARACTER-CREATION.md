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

1. **Race & Gender**: select race (the nine species included in SRD 5.2.1)
   from the two-column list and gender from the dropdown below it. Gender does
   not alter stats or restrict other choices. Both selections persist when returning.
2. Select alignment.
3. The **Dice Rolls** area starts with six empty boxes to the right of the
   abilities. Roll attributes to fill those boxes with totals only. The native
   character retains the underlying dice. Ability boxes remain empty until you drag
   results into them. Assigned results leave their original boxes empty.
   Dragging between filled ability boxes swaps their results. Dropping an
   unassigned roll onto a filled ability returns the displaced result to the
   right. All six abilities must be assigned before continuing. **Reroll all six**
   replaces the complete set and empties ability boxes; attempts are unlimited.
   Choose one of the four SRD backgrounds here, then allocate its attribute
   bonuses (+2/+1 to different allowed abilities, or +1 to all three).
   Assigned ability boxes include those bonuses and update immediately when
   either selector changes, even before all six rolls are assigned. Empty boxes
   remain empty; the unassigned dice retain their original totals.
   Each applied adjustment appears beside its ability (for example,
   `Soldier (+2)`). Only the ability score is colored: yellow above
   its original roll, red below it, and normal when unchanged. Dice boxes have
   room for a two-digit total and padding, with a drag hint beside **Dice Rolls**.
4. Select one starting class from the twelve SRD classes. Classes whose primary
   abilities do not meet the SRD multiclass prerequisite of 13 are disabled.
   Fighter requires Strength or Dexterity; Monk and Ranger require Dexterity
   and Wisdom; Paladin requires Strength and Charisma. Applying these minimums
   to the initial class is the requested OpenGold house rule, not an SRD
   level-one restriction. Background bonuses count toward eligibility.
   Return to Attributes to change assignments or bonuses if needed.
5. Enter a name, up to 40 characters. There is no separate HP step.
   Choose a portrait head from the dropdown or browse with the previous/next
   buttons directly below the portrait, then choose a body with the second row
   of arrows. These controls are available on every creation step, including
   the finished sheet, and are disabled after **Add to party**. The ready/action
   previews sit below the controls and scale down at the minimum window size.
   The list includes all original heads plus male
   and female Gnome, Orc, Goliath, Tiefling and Dragonborn heads. Race/gender
   selections initially suggest the matching new head. Manually choosing a
   head keeps it selected through later edits; every head remains available.
   With no exact match (including Nonbinary), the initial original head remains
   the default until you choose one. **Start over** restores automatic defaults.
6. Customize combat head and weapon/body parts, tall/short art, and all twelve
   region colors. Select a region's Color-1 or Color-2 button, then a palette
   swatch. Enlarged ready and action previews update immediately, recoloring
   only that part, with the composed portrait head and body visible above both
   poses. Controls show **Not present** when the selected parts omit
   that region in both poses. Its saved colors return when the part is present.
7. Show the character sheet: race, gender, class, level, background, six scores,
    saving throws, Hit Dice, HP and inventory (initially empty). Saving throws
    include the class's level-one proficiency bonus where applicable.
    A note explains that SRD 5.2.1 uses the maximum class Hit Die plus applicable
    modifiers at level one. Only the HP numbers are colored: yellow for a
    positive combined modifier, red for a negative modifier, normal for zero.
    **Modifiers** opens a modal with ability score adjustments, racial, class, background, equipped
    item and spell details, identifying the source of each applied modifier.
    Its ability section shows changes to the rolled scores, not derived bonuses.
    Unadjusted abilities are omitted. Each adjusted ability starts with its rolled
    score, followed by each source with its signed adjustment in parentheses,
    then the final score on a separate line. The dialog scrolls for longer content.
    **Saving Throws**, beside Modifiers on both sheets, opens a separate dialog
    with an editable target DC (initially 15), the required d20 roll for each save,
    and ability-score and class-proficiency sources. Invalid DC input shows a
    correction hint. Rolls that always meet or cannot reach the DC are identified;
    ordinary saves do not automatically fail on 1 or succeed on 20. Conditional
    racial traits and persistent spell effects are not implemented in these totals.
    Score numbers are yellow for positive ability modifiers, red for negative
    modifiers, and the normal text color for zero. Unimplemented effects are
    identified explicitly.
    The party screen uses this same sheet for its selected member, including
    live HP, equipment and resources; roster entries show character names.
    Ready and Action combat sprites appear side by side under the party portrait,
    with their labels below the images. Portrait selection has no separate step;
    its preview controls remain available throughout creation.

**Back** preserves selections and allows earlier edits. Derived scores and HP
update from the current choices. **Start over** clears the single character.

The [shared party preview](PARTY.md) now adds a roster, six PC/two NPC positions,
party sheets/inventories, town purchases/equipping and combat handoffs to this
scene. Character creation itself retains the same choices. Supported combat
profiles and remaining class/skill/spell/lineage limits are listed there.
Persistence remains separate work.

## Native boundaries and rules

- `OpenGold.Rules/character_rules.h` defines the optional `CharacterRules`
  capability, draft, retained dice, choices, adjustments and sheet values.
- `OpenGold.Rules.Srd5` supplies the SRD choices and arithmetic. It has no Godot,
  original-asset, or campaign dependency. Dice use seeded SplitMix64 with
  rejection sampling; invalid assignments and selections are rejected.
- `OpenGold.Core/character_creator.h` owns the injected rules module and the
  single character's sequence and appearance. A completed sheet must return to
  editing before changing choices or scores; appearance can change during sheet
  review. No rolled result can be assigned twice.
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

Saving-throw proficiencies follow the SRD's Core Class Traits tables; the native
rules layer adds +2 to the two proficient saves at level one. The modifier modal
reports the effects currently implemented by that rules layer, including active
equipment conversions. It does not imply full species traits or ongoing spell
effect support.

## Original character art

`CharacterArt` in `OpenGold.Core` reads the installed game's DAX files, retaining
archive IDs and indexed combat pixels. No extracted artwork is distributed.

- Portrait heads use `HEAD1..8.DAX`; bodies use `BODY1..8.DAX`. The loader merges
  identical copies of an ID and rejects conflicting copies. It exposes the
  available original parts, without claiming a race/class-specific selection
  list. A head is 88 x 40 pixels, followed by an 88 x 48 body.
- The ten approved [OpenGold heads](../data/art/portraits/README.md) are loaded
  in addition to the original heads. Their stable IDs 256..265 do not overlap
  the original byte-sized DAX IDs and are retained in `CharacterAppearance`.
  `build-rolf.cmd` copies their source PNGs into `godot/bin/portraits/`;
  `review-character.cmd` imports them through Godot's resource system.
  The native core fits the decoded images to 88 x 40 with nearest-neighbor
  sampling, trims bottom padding, and crops at a measured neck baseline.
  Each head has neck anchors; composition centers them on the selected body's
  skin opening and tapers only the lowest five rows to match its width. Changing
  bodies immediately refits the join. Faces and horns are translated without
  horizontal stretching; the source PNGs and original head/body pixels stay intact.
  Approved colors are preserved without forcing the new heads into the EGA
  palette; original body art and combat icons keep their existing colors.
  These files are needed for the demo; a missing resource reports its filename.

### Portrait neck alignment

Local inspection of 41 original heads and 21 bodies found that most `HEAD`
bottoms occupy x=36..55 (inclusive) on the 88-pixel grid. The narrower bodies
use openings such as x=36..51 or x=37..54. The compositor identifies the selected
body's opening by its original skin color `(255,85,85)` in top-row columns
30..61, excluding collars and armor. An unrecognized opening uses x=36..55.

`AdditionalPortraitHead` stores the source neck edges and retained row count
on the fitted 88 x 40 grid. All ten heads now keep all 40 rows. The male Orc,
Goliath, Tiefling and Dragonborn source artwork was revised to give the necks
straight sides and flat bases, with the complete chin above the join. Their
anchors were remeasured. In particular, the old Orc crop removed needed neck
space and is no longer applied. Anchors exclude braids and hair from the neck
measurement; original body skin colors can still differ from the new heads.

Native tests check varied neck widths, unchanged face proportions/body pixels,
and retention of the revised necks. The Godot `--character-check` also checks
every new head against every loaded original body and verifies live updates
when changing bodies. Numerical seam checks cannot establish anatomical fit;
the armor comparison capture is also reviewed visually.

### Combat parts

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
transparency, character data ownership, inventory operations, additional-head
IDs, panel fitting, approved color preservation, and body composition. Set
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
portrait/parts selection, all ten new heads, race/gender defaults, all twelve
colors, sheet review and restart. Each
palette click checks the preview texture pixels against the composed icon and
counts changed pixels in each pose; it also checks that the portrait is intact.
The completed sheet is checked against the reusable character and inventory. Buttons
and list selections use injected viewport mouse input; name entry uses keyboard
events. Background controls also exercise their native selection signals.
For local screenshots of attributes, appearance and the sheet, omit
`--headless` and append `--capture`. Captures go to ignored `user-data/` files.
This also saves one composed portrait preview for each new head as
`character-portrait-<species>-<gender>.png`.
`character-portrait-armor.png` compares original head 1 with the male Orc,
Goliath, Tiefling and Dragonborn (columns left to right) on armor bodies 1, 18
and 26 (rows top to bottom). It is a local rendering of installed game art and
is not distributed.

## Future class planning foundation

The native draft retains multiple desired class IDs separately from its one
level-one class. Target eligibility can be queried with partial assignments;
unmet targets never prevent selecting a different qualified starting class.
On Attributes, compact rolled totals sit beside the ability boxes. The scrollable
**Target class(es)** checklist shows all twelve classes and their prerequisites.
Checkboxes record future goals even when the character is not yet qualified.
Unmet requirements turn the relevant score box dark red and show a readable
explanation immediately below it. Warnings update with targets, assignments,
swaps, rerolls, backgrounds and bonuses; Fighter's either/or condition clears
both warnings when either primary qualifies. The character sheet retains these
future goals separately from the starting class.
SRD 5.2.1 multiclassing acquires another class when gaining a level. Later
Ability Score Improvement features can raise scores and help meet prerequisites;
future increases are not applied to level-one scores.

Rules reference: [SRD 5.2.1](https://media.dndbeyond.com/compendium-images/srd/5.2/SRD_CC_v5.2.1.pdf),
Multiclassing and class Ability Score Improvement features.
