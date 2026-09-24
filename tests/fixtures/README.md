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

### Adrenaline Rush introduction (0.6.14 writer)

`campaign-v10-adrenaline.ogs`, `combat-v11-adrenaline.save` and
`combat-v11-adrenaline-continued.save` were generated from commit **efbe48d**
before the #197 writer changed. Source and static libraries were frozen under
`/tmp/opengold-adrenaline-legacy`; its generator checked module version 0.6.14.
These are five level-two normally created characters (alternating Orc and Dwarf,
one Wizard and four Fighters), including an Orc reserve. They retain wounds,
spent Wind/slots/Hit Dice, zero-HP recovery deadlines, a deceased character and
Blinded. The first Orc also has 7 Temporary HP from `spell:fixture`. The campaign
uses time 1234 minutes plus 5678 ms and an earlier individual Long Rest.

The combat pauses before actor 4 leaves an enemy's reach; the continued file
records the old writer's Decline result. Migrating both must produce the same
continuation without changing RNG, time, pool, existing resources or Dwarf
grants. Orc use capacity is new and starts full; the campaign independently
checks the exact resulting SRD7 state. Do not regenerate these files with the
current writer.

## Heavy weapon requirements (0.6.15 writer)

`campaign-v10-heavy.ogs`, `combat-v12-heavy.save` and
`combat-v12-heavy-continued.save` were generated with frozen source and static
libraries from commit **42a7f6c**, module **0.6.15**, before #55 changes. The
generator checked that version. Do not regenerate these with the current writer.

The campaign contains a Dwarf Fighter (Strength 12, Dexterity 15, Greatsword)
and an Orc Wizard (Strength 15, Dexterity 12, Longbow), both authored through
normal character rules with selected languages. Second Wind, Magic Missile and
Adrenaline Rush have spent resources; the Orc retained 7 Temporary HP from
`spell:fixture` through Keep current. Existing PC9 species grants must survive
migration. Asset identity is `heavy-fixture`.

The combat uses seed 1 and pauses an enemy's movement out of the Dwarf's reach.
The continued file records Decline. Migration changes only module identity and
must reproduce that exact continuation. Choosing Opportunity Attack instead
now correctly selects 11 over 16 with Disadvantage; the old writer omitted that
penalty. New save/restore continuation must agree on the corrected outcome.

## Complete weapon catalog (0.6.16 writer)

`campaign-v10-catalog.ogs`, `combat-v12-catalog.save` and
`combat-v12-catalog-continued.save` were generated by the frozen source/static
libraries of commit **93d3c32**, module **0.6.16**. The generator checked its
module identity; do not regenerate with the current writer. Asset identity is
`catalog-fixture`.

Three normally authored characters carry the old `longbow` key: a Dwarf Fighter
with original type-45 provenance (named Heavy crossbow by the fixture's legacy
generic label), an Orc Wizard with Longbow/type-43 provenance, and a Human
Fighter with original type 45 but no source record. All three retain Longbow:
the campaign's reviewed type-45 interpretation is Fine Composite Long Bow, and
a display label alone is not evidence for changing an original conversion.
All have wounds; the Dwarf has one spent Wind, while the Orc has one spent slot,
one spent Rush and 7 Temporary HP. Time is 123 minutes plus 456 milliseconds.
Campaign migration preserves the original resource descriptions and changes
only module identity.

The two combat files record seed 13 before and after one ranged attack. Combat
recipes lack original-item provenance, so they retain their recorded Longbow
and match the old writer's exact result, resources and random continuation.

`weapons-srd-5.2.1.tsv` is a separate, source-verified table of all 38 weapons
from SRD 5.2.1 p.91. Tests compare every definition/property, including exact
quarter-pound/copper units, without reading expected values from implementation.

## Armor catalog compatibility

`campaign-v10-armor.ogs`, `combat-v12-armor.save` and
`combat-v12-armor-continued.save` were written by **0.6.17**, using headers and
compiled libraries frozen from commit `5dfabb7`. Do not regenerate these with
current code. Asset identity is `armor-fixture`; there are no original assets.
Three normally created characters (Dwarf Fighter, Orc Wizard, Human Fighter)
carry Longswords. The Fighters wear Chain Mail; the Wizard wears untrained
Leather. The first two armor items retain authored original type 55/50 source
records. Everyone is wounded; Second Wind, slots and Adrenaline Rush have
spent uses, the Orc has sourced Temporary HP, and the clock is 123 minutes plus
456 milliseconds. The combat uses seed 13 and records an actual melee attack
continuation. Migration changes only module identity; it must not rewrite
resources, descriptions, equipment, grants, RNG or clocks.

`armor-srd-5.2.1.tsv` is an independent transcription of the SRD 5.2.1 p. 92
table. Its thirteen rows cover twelve suits and Shield. Times are seconds;
Shield's zero times denote a Utilize action, not a free equipment change.
