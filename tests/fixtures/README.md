# Save migration fixtures

`campaign-v1.ogs` and `campaign-v2.ogs` retain the earlier authored campaign
formats used by the save tests.

`campaign-v6-low-con.ogs` was written by rules module 0.6.2, built from commit
`c59ce07`, before the HP-history correction. Do not regenerate it with the
current writer. It contains six level-four Wizards created through the normal
character/advancement APIs, with Sage bonuses excluding Constitution and a
level-four +2 Constitution increase:

| Member | Species | Starting Constitution | Old maximum/current HP | State |
| --- | --- | --- | --- | --- |
| 1 | Human | 3 | 6 / 6 | Healthy |
| 2 | Human | 3 | 6 / 4 | Wounded |
| 3 | Human | 3 | 6 / 0 | Unconscious, one success/two failures |
| 4 | Human | 3 | 6 / 0 | Dead, one success/three failures |
| 5 | Dwarf | 3 | 10 / 8 | Wounded |
| 6 | Human | 15 | 30 / 28 | Wounded; unaffected by the correction |

All have one remaining slot at each spell level. Migration must produce maxima
9, 9, 9, 9, 13 and 30; current HP 9, 7, 0, 0, 11 and 28; and preserve the resource
strings and dead flag. Subsequent saves must reload without applying the
correction again. The fixture uses no original game assets.


## Combat opportunity-reaction migration

The three `combat-v5-*.save` fixtures were written by rules **0.6.4** from
commit `1c75229`, before removal of facing-triggered reactions. Preserve the
original writer bytes, including trailing spaces. They contain no original assets.

- `combat-v5-facing.save`: a Vanguard at (2,3) has turned left and attacked.
  Its action and Bonus Action are spent, Second Wind is exhausted, HP is 20,
  and 25 feet of movement remain. Guards 3 and 4 await facing-only reactions.
  Migration cancels that queue and changes ticket 4 to 5, preserving every
  other actor, log, RNG, effect and clock field.
- `combat-v5-movement.save`: the same wounded/healed actor attacked to the right,
  then tried to move from (2,2) to (1,2). Guard 3 already declined; guards 4 and
  2 remain queued before the step. Migration must preserve that exact queue.
- `combat-v5-movement-resolved.save`: the old writer's reference continuation
  after guards 4 and 2 take their opportunity attacks. Both miss, movement
  finishes at (1,2), and 25 feet remain. New rules must match all actor, log,
  RNG, clock and resource fields after applying the same commands.

## Versatile grip migration

`campaign-v6-grips.ogs` and `combat-v7-grips.save` were written by the **0.6.6**
libraries built from commit `5daabbb`, using that commit's headers. Do not
regenerate them with the current writer. They contain no original assets.
Members 1–7 carry Quarterstaff, Spear, Battleaxe, Trident, Longsword, Warhammer
and War Pick respectively; member 7 is a recruited NPC. The old hand use is
two, two, two, two, one, one, one. Member 1 is wounded at 5 HP with one Second
Wind remaining. The combat seed is 42 and Battleaxe wielder 3 acts first.
The campaign asset identity is `grip-fixture`. Migration preserves hand use,
wounds and recovery, introduces the corrected two-handed melee dice, and
retains deterministic subsequent saves and combat commands.

## Feature and feat grant migration

`campaign-v7-grants.ogs`, `combat-v8-grants.save` and
`combat-v8-grants-continued.save` were written by rules **0.6.7** from commit
`abda7ea`, using its headers and compiled libraries. Do not regenerate with the
current writer. The campaign asset identity is `grant-fixture`; all data is authored.

| Member | Class/background | Level | Level-four choice | Max/current HP | AC |
| --- | --- | --- | --- | --- | --- |
| 1 | Fighter/Soldier | 1 | None | 12/9 | 16 |
| 2 | Fighter/Soldier | 4 | +2 Constitution | 40/37 | 16 |
| 3 | Fighter/Sage | 4 | Defense | 40/37 | 17 |
| 4 | Fighter/Sage | 4 | Savage Attacker | 40/37 | 16 |
| 5 | Dwarf Wizard/Sage | 4 | +2 Constitution | 38/35 | 12 |
| 6 | Cleric/Soldier | 3 | None | 24/21 | 16 |

Fighters have one Second Wind remaining; the casters have one slot of each
available level remaining. Everyone carries a Longsword; all except the Wizard
wear Chain Mail. The combat seed is 42. Its continued reference applies twelve
commands, choosing the first legal melee attack or otherwise End Turn. Migration
must preserve the exact resulting damage, per-turn feat usage, resource/turn
budgets, RNG and log. Campaign history identifies the grants' sources; the older
combat recipes only encode effects and must not acquire fabricated provenance.

## Training migration

`campaign-v8-training.ogs` and `combat-v8-training.save` were written by rules
**0.6.8** from commit `58e8955`, before training choices existed. The original
headers and compiled libraries produced these files; do not regenerate them with
the current writer. The authored campaign uses asset identity `training-fixture`.
Its four Human characters are Rogue/Criminal level 1, Rogue/Soldier level 1,
Fighter/Soldier level 4 with Defense, and Wizard/Sage level 4 with +2 Constitution.
All are wounded by 2 HP; the Fighter has one Second Wind and the Wizard has one
slot at each spell level remaining. Combat seed 42 retains their original PC6
recipes and state. Migration must not invent optional training choices, heal
wounds, refill resources or rewrite the paused combat recipes.

## Rest resource migration

`campaign-v9-rest.ogs`, `combat-v8-rest.save` and
`combat-v8-rest-continued.save` were written by rules **0.6.9** from commit
`7b55636`, before spendable Hit Dice existed. The original libraries and headers
produced these files; do not regenerate them with the current writer. They use
asset identity `rest-fixture` and authored Human characters: level-four Fighter,
Wizard and Cleric, plus level-one Rogue/Criminal. All training is selected. The
Fighter uses Defense and a Longsword in two hands; both casters increased
Constitution at level four. Each character is wounded by 5 HP. Fighter has one
Second Wind, Wizard one slot of each level and an active Blinded effect, Cleric
two level-one slots and no level-two slots. Campaign time and prior rest offsets
are nonzero. The combat starts with the Fighter's departure from the enemy's
reach; the second file resolves that pending opportunity attack using the old
writer. Migration must preserve every original field and outcome while adding
only unspent Hit Dice to format-nine combat actors.

## Recovery-clock migration

`campaign-v10-recovery.ogs`, `combat-v9-recovery.save` and
`combat-v9-recovery-continued.save` come from commit `1adfc63`, module **0.6.10**.
A frozen copy of that commit's headers, content and built libraries generated
these files; do not regenerate them with the current writer. Asset identity is
`recovery-fixture`. Five authored level-two Human PCs include a Stable Fighter
with one spent Hit Die/Second Wind, an unstable Wizard with two successes/one
failure and Blinded/spent slots, a dead Fighter, a conscious Fighter and an
unstable reserve. Campaign time is 1234 minutes plus 5678 ms, with a prior rest
completion on the Stable Fighter. All language choices are completed.

The combat contains the three living active PCs and an authored enemy. Initiative
order is IDs 4, 1, 99, 2. It pauses on Fighter 4's departure from enemy reach; the
second file declines the reaction without elapsed time. Old data has no recovery
clocks. Migration adds a known 4500 ms until Wizard 2's next initiative slot and
a deferred legacy recovery roll for Stable Fighter 1, preserving every other
field and the old writer's pending-movement continuation. Campaign migration
retains its original opaque states without a load-time roll.

## Typed damage and Dwarf resistance migration

`campaign-v10-damage.ogs`, `combat-v10-damage.save` and
`combat-v10-damage-continued.save` were generated by the frozen **0.6.12** writer
from commit `4557cf2`, before any F05 code/content changes. A separate copy of its
headers, content and built libraries generated them; do not regenerate with the
current writer. Asset identity is `damage-fixture`. The five level-two members
alternate Dwarf/Human, with a Stable Dwarf's rolled 4321000 ms remaining and one
spent Hit Die/Second Wind, an unstable Human Wizard with Blinded/spent slots, a
dead Dwarf, a conscious Human and an unstable Dwarf reserve. All languages are
selected. The combat pauses on member 4's departure from enemy reach and its
continued file declines that reaction. New campaign reconstruction adds only the
fixed species resistance; combat continuation retains the prior writer's actor
rows/recipes, timers, resources, RNG and movement, replacing only the identity.

## Temporary HP migration

`campaign-v10-temporary-hp.ogs`, `combat-v10-temporary-hp.save` and
`combat-v10-temporary-hp-continued.save` were generated by the frozen **0.6.13**
writer at commit `eea7368`. A separate copy of its headers, content and built
libraries generated these before the Temporary HP changes. Do not regenerate
them with the current writer. Asset identity is `temporary-hp-fixture`.
They use the same five-member mortality/resource scenario as the damage fixtures,
but contain validated PC8 Dwarf resistance grants. None has Temporary HP because
the old writer did not support it. Combat migration appends only the empty pool
to each actor and updates format/module identity; declining the saved opportunity
decision matches the frozen continuation. Campaign migration changes only module
identity and retains all existing state.
