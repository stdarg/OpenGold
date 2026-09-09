# Manual advancement

When a living Fighter, Cleric or Wizard has enough XP, a small **↑** button appears
beside the character's name in the party roster and idle exploration party list.
XP alone never changes a level. Click the arrow to preview the next level's HP,
select supported spells, and choose a feat or ability points at level 4.
**Confirm** applies one level; **Cancel** and closing the dialog change nothing.
If another level is available, the arrow remains. Advancement is blocked during
combat and unfinished exploration events.

This increment supports single-class levels 2–4, with thresholds of 300, 900 and
2,700 total XP. HP uses the fixed average after first level, includes retroactive
Constitution changes and preserves the existing HP deficit. Existing resource
expenditure is preserved; only new capacity is added. An unconscious character
does not become conscious merely by leveling. No XP is deducted.

## Available choices

- **Ability points:** at level 4, add two points to one ability or one point to
  two abilities, with a maximum score of 20. Scores, modifiers, saves and derived
  combat values are rebuilt together.
- **Defense:** level-4 Fighter choice; +1 AC while wearing armor.
- **Savage Attacker:** level-4 choice for a character who does not already have
  it from Soldier. The first successful weapon attack each turn uses the higher
  of two damage rolls. Soldier's existing background feat now uses this mechanic.
- **Cleric:** Cure Wounds and Healing Word. At least one must be selected.
- **Wizard:** Magic Missile, plus Scorching Ray from level 3. Fire Bolt remains
  available without spending a slot. At least one leveled spell must be selected.

The spell list is a curated subset, not the complete class preparation or
spellbook system. Unavailable examples (Bless, Shield, Grappler and Magic
Initiate) are disabled and explain their missing mechanics in a tooltip.

Level-one slots are 2/3/4/4 at character levels 1/2/3/4. Level-two slots are 2 at
level 3 and 3 at level 4. **Slot level** in combat switches between level-one and
level-two casts for Cure Wounds, Healing Word and Magic Missile. Scorching Ray
always spends a level-two slot. Healing Word uses the bonus action and heals
2d4 plus the casting modifier; its level-two cast heals 4d4. Cure Wounds uses
2d8/4d8. Magic Missile fires three/four darts. Scorching Ray makes three separate
ranged spell attacks for 2d6 each. Only one spell slot may be spent per turn.

Damage spells currently direct all darts/rays at one target; split targeting is
unavailable. Full class/subclass features, Action Surge, Channel Divinity,
concentration, broader spells/feats and multiclass advancement remain outside
this increment. The dialog states the class/subclass limitation.

Rules use the [official SRD 5.2.1](https://media.dndbeyond.com/compendium-images/srd/5.2/SRD_CC_v5.2.1.pdf),
including its current Healing Word and Savage Attacker mechanics. Existing pack
attribution applies; original game resources are loaded locally.

## Saves and verification

Campaign format 3 records each confirmed choice and reconstructs the resulting
sheet through the rules module. Formats 1 and 2 migrate their existing levels
using the previous default choices. Rules module 0.4.0 accepts the precise
supported 0.3.0 campaign identities; unrelated content identities still reject.
Standalone combat checkpoints use version 3 to retain second-level slots and
per-turn spell/feat usage. Old-module combat checkpoints are not migrated.

From PowerShell:

```powershell
.\build-rolf.cmd
.\review-character.cmd --advancement-check
.\review-character.cmd --level-up-review
```

The automated check exercises real roster and town buttons, Cancel, invalid
points, HP preview, spell/feat confirmation and disappearance of the arrow at
the level cap. It writes local captures under `user-data/level-up-*.png` and exits.
The review command opens an isolated three-character level-3 fixture with enough
XP for level 4, then leaves the controls available for manual inspection. Its
saves use a separate profile under `user-data/level-up-review-profile`.

Native advancement tests cover all three classes, transactional rejection,
Constitution HP recalculation, Defense AC, Savage Attacker damage, spell action
budgets, second-level resources and campaign/combat save reconstruction. Frozen
authored format-1/2 fixtures verify migration independently of the current writer.
