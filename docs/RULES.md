# Replaceable combat rules

The selected baseline is **SRD 5.2.1**, with standard turn-based combat and spell
slots. Open5E supplies reference content; it is not an executable rules engine.
The first implementation is a bounded C++20 rules module and a native Godot
combat scene. It is not a complete implementation of the SRD.

The [shared party preview](PARTY.md) extends this module with created-character
recipes, equipment and persistent vitals/resources. The standalone combat modes
below retain their fixed fixtures; the character scene injects the actual party.

## Run from Windows CMD

From the repository root:

```cmd
build-rolf.cmd
review-combat.cmd
```

The existing build helper builds both native Godot scenes and runs five test
suites. Close any running OpenGold scene before rebuilding its shared DLL.
See [native build prerequisites](ROLF.md#build-boundary).

Training uses one martial fixture against an SRD Bandit and needs no original
game files. Select **Slums event** for a fixed four-person party against the
original event's four orcs. Original files come from `OPENGOLD_GAME_DIR`, falling
back to `opengold/game_directory` in `godot/project.godot`:

```cmd
set "OPENGOLD_GAME_DIR=C:\Games\POOLRAD"
review-combat.cmd -- --slums
```

Select an action, then click a highlighted destination or target. Dash, Dodge,
Disengage, Second Wind, and reactions execute directly. **Enter** ends your turn
or continues a dialogue pause. Enemies act automatically. The roster is in
initiative order; numbers identify tokens. The active actor's remaining movement
and resources appear above it. Gray cells block movement and sight; brown cells
cost extra movement. Each square is 5 feet, including diagonals.

**Restart** resets the selected isolated encounter with seed 42. After Slums
victory, **Continue** finishes any remaining dialogue, and **Revisit event** checks
the original event's persistent flag within this session. This scene is separate
from Rolf's exploration scene; walking into combat from exploration is future work.

Training **Save combat** / **Load combat** use `user-data/combat.save`, retaining
the previous save as `.bak`. Saves include pending reactions and RNG state and
reject a different module or content fingerprint. Slums campaign saving remains
disabled until ECL, party, and combat can be persisted together.

## Library boundary

| Component | Responsibility |
| --- | --- |
| `OpenGold.Rules` / `opengold_rules` | Edition-independent encounter, snapshot, legal-command, outcome, and checkpoint interfaces. |
| `OpenGold.Rules.Srd5` / `opengold_rules_srd5` | SRD combat calculations, turn/resource state, dice, validation, and serialization. Links only the rules interface and the C++ standard library. |
| `OpenGold.Core::CombatDemo` | Supplies authored geometry and explicit creature conversions; pauses/resumes the original ECL host request around real combat. Depends on the interface, not SRD code. |
| `OpenGold.Godot::CombatView` | Draws state and submits offered commands. The application composes the selected module. It never rolls dice or computes damage. |

`RulesModule::create(encounter, seed)` returns an owned `CombatSession`.
`snapshot()` provides display state; `legal_commands()` provides the actions the
module currently permits. `submit()` validates actor, revision, verb, target and
destination again. Invalid or stale input changes neither state nor randomness.
`save()` returns module-owned checkpoint bytes; `restore()` validates their
identity and structure before replacing a session.

Modules are replaceable at C++ composition/build time. There is no dynamic plugin
ABI or user-facing edition selector yet. A new implementation supplies the same
interface, its content and conversion profiles. Verbs are strings owned by the
module. The current demo presents the listed actions explicitly; a replacement
introducing new verbs also needs presentation controls for them. The common
interface does not expose SRD spell slots or class internals as mutable state.

Sessions exclusively own mutable state and share immutable definitions through
`shared_ptr<const Content>`. They can outlive the module that created them.
Deterministic SplitMix64 dice and stable initiative tie ordering make a seed plus
the same accepted command sequence reproducible. Checkpoints include the RNG,
turn budgets, HP, slots, death saves and unfinished opportunity reactions.

## Implemented scope

- Individual initiative; ties resolve by entity ID. Action, Bonus Action,
  Reaction, movement, and round/turn reset.
- Grid pathfinding, difficult terrain, allied transit, blocked enemy cells,
  opaque obstacles and blocked diagonal wall corners. Movement can pause for an
  opportunity attack before leaving reach; Disengage prevents it.
- One melee or ranged attack per Attack action, ascending AC, natural 1/20,
  doubled damage dice on critical hits, Dodge and ranged disadvantage from long
  range or an adjacent visible enemy. No hidden dice in the UI or AI.
- Dash; Second Wind; Fire Bolt; touch-range Cure Wounds; single-target Magic
  Missile. Two level-1 spell slots in the caster fixtures. Cure Wounds uses
  2d8 plus the casting ability modifier from this baseline.
- HP, unconscious party members, automatic death saves, stabilization, healing,
  instant death from excess damage, enemy defeat, and party incapacitation.
- Complete training checkpoints and a basic AI that uses the same public
  commands as the player.
- The [campaign recovery subset](RECOVERY.md) advances Fighter/Cleric/Wizard
  to level 2 at 300 XP, using fixed-average HP growth and updated caster slots.
  Higher advancement stops explicitly. Eligible eight-hour long rests restore
  supported resources; repeated rests require a 16-hour wait. Stable campaign
  reward IDs survive scene recreation and native checkpoints.
- Temple Cure Wounds is a 100 gp atomic service for a wounded living active
  member. Dead targets reject; no resurrection is implied.

The fixed profiles are intentionally limited to levels 1–4 and ordinary Medium-sized
ground combatants. Created campaign profiles support the documented level 1–2
subset. Standalone party profiles are authored combat fixtures, not finished
character sheets. The orc conversion is authored for this demo; its AC/HP are
not an automatic conversion of original AD&D values.

Not yet implemented: full class features or equipment,
weapon mastery, Extra Attack, regular ability saving-throw effects, general
advantage/condition handling, partial cover, prone/grappling, damage
types/resistance, multiple sizes, concentration, other spells, multiple spell
levels/upcasting, split-target Magic Missile, retreat, morale, or complete
campaign encounter coverage. The shared campaign now includes named saves,
surprise initiative disadvantage, original dungeon geometry and bounded original
loot; see [the expedition adapter](EXPEDITION.md) and [campaign saves](SAVES.md).
Unconscious enemies are currently treated as defeated, and attacks against
unconscious party members are not offered. Allied transit and diagonal geometry
use the documented grid adjudication; this is not an implementation of every
SRD movement exception. Unsupported creature definitions fail explicitly.

## Original Slums adapter

This is the isolated, tested `ECL2.DAX:20` search-entry-1 profile, with `C04F=1`.
It preserves original text, the one-choice menu flow, monster/count/icon operands,
and result-dependent bytecode continuation. It acknowledges the three setup and
presentation services used here; full encounter presentation is still pending.

| Original input | Current conversion |
| --- | --- |
| `LOAD MONSTER 13, 1, 4` | `MON2CHA.DAX:13`, one original ORC, `slums-orc` rules definition, `CPIC2.DAX:4` pose 0 |
| `LOAD MONSTER 4, 3, 4` | `MON2CHA.DAX:4`, three original ORCs, same explicit rules/art mapping |
| `COMBAT`, `6DC6=99`, `6DCB=0` | Fixed four-person party, four original enemies, authored 12×9 arena |
| Victory / defeat | `6DC7=0` / `128`; `6DC8` is the actual defeated-monster count |

Bank 2 is explicit in this event profile; general script-to-resource bank
resolution is not inferred. Raw original creature records remain unchanged in
`CreatureCatalog`. Combat icons are decoded locally through the native CPIC
decoder; absent CPIC art falls back to a numbered token. Original dialogue or
art is never included in the repository.

The actual fight result resumes the waiting COMBAT request and clears its
documented scratch fields. With the current fixture and seed, victory leaves
`4ACA=255`, `4ABB=1`, and `6DC8=4`. A second visit does not start another fight.
This verifies the original event state in memory, not campaign persistence or
post-combat treasure. Unknown service contexts fail rather than being simulated.

## Validation and next increments

`build.cmd` tests the rules without Godot; `build-rolf.cmd` also builds the scene.
With `OPENGOLD_GAME_DIR` set, `opengold_rules_tests` executes the installed Slums
script, fights using actual legal commands, checks result writes and revisits.
The older ECL mock remains a separate bytecode regression test.

Native tests cover command rejection/RNG atomicity, action budgets, obstacle
targeting, opportunity reactions, healing and spell resources, unconsciousness,
critical attacks, checkpoint validation and deterministic continuation. Synthetic
CPIC tests cover the format separately from installed game assets.

Godot scene checks use the actual native action-button signal and target-input
path, prevent Enter from skipping enemy turns, and finish combat automatically:

```cmd
godot --headless --path godot res://scenes/combat_demo.tscn -- --combat-check
godot --headless --path godot res://scenes/combat_demo.tscn -- --combat-check --slums
```

Add `--capture` to the user arguments in a graphical run (omit `--headless`) to
save a local PNG in `user-data`. These checks exit automatically.

Next increments should add one tested encounter's required mechanics at a time:
regular saving throws and conditions, campaign party/encounter persistence,
exploration-to-combat transitions and original battlefield geometry, then rewards
and wider creature/spell coverage. Full character creation follows the fixed
party milestone. The [standalone character creator](CHARACTER-CREATION.md) now
implements the requested identity, attribute, HP, and appearance flow through
a separate optional native `CharacterRules` capability; it does not replace
combat fixture definitions. Reverse engineering remains focused on campaign interfaces,
file formats and content mapping rather than recreating the original combat rules.

Reference content and attribution are recorded in
[the versioned pack](../data/rules/srd-5.2.1/README.md). The baseline is the
[official SRD 5.2.1](https://www.dndbeyond.com/srd).
