# Rest resources

[F03a #190](https://github.com/stdarg/OpenGold/issues/190) implements Hit Dice and
supported rest recharge in the rules layer. Parent [F03 #30](https://github.com/stdarg/OpenGold/issues/30)
remains open until the [campaign flow #191](https://github.com/stdarg/OpenGold/issues/191)
and [controls #192](https://github.com/stdarg/OpenGold/issues/192) are delivered.
The ordinary game does not yet offer Short Rests or Hit Dice spending controls.

## Rules and scope

Authority: [SRD 5.2.1](https://media.dndbeyond.com/compendium-images/srd/5.2/SRD_CC_v5.2.1.pdf),
pp. 47–48 (Second Wind), 185 (Long Rest) and 187 (Short Rest).

- Characters have one Hit Die per attained level: d12 for Barbarian; d10 for
  Fighter, Paladin and Ranger; d6 for Sorcerer and Wizard; d8 for the other six
  classes. Supported progression remains levels 1–4 for Fighter, Cleric and
  Wizard, and level 1 for the others.
- A single-die spend rolls that die and adds current Constitution, restoring at
  least 1 HP before the maximum-HP cap. Each call commits one RNG draw so another
  spending decision can follow its result. A voluntarily spent die is consumed
  even at full HP. Dwarven Toughness does not add to the healing roll.
- A completed Short Rest restores one spent Second Wind use, up to capacity.
  It restores neither Hit Dice nor ordinary Cleric/Wizard slots. Spending zero
  dice still allows feature recharge.
- A completed Long Rest restores all spent dice, HP and supported slots/Second
  Wind uses. Existing campaign timing remains eight hours with a sixteen-hour
  delay after the previous completion before another Long Rest can begin.
- Recovery requires a living character with at least 1 HP. Invalid state and
  exhausted dice reject without changing vitals or RNG. Advancement adds the new
  level's die without replenishing earlier expenditure.

Other class pools, multiclass die mixtures, exhaustion, maximum-HP reductions,
species-specific rest features and interruption/resumption remain their named
issues. This delivery does not claim individual campaign rest eligibility or a
player-facing spending flow. See [existing recovery boundaries](RECOVERY.md).

## Architecture and API

`RulesModule::recovery_info` reads die size, remaining/maximum dice, vitality
eligibility and named supported resource pools. Each pool reports its capacity,
remaining uses and Short Rest recharge amount. `can_rest` checks vitality only;
activity and cooldown checks belong to the campaign.

`recover_short_rest` applies recharge after the caller establishes completed-rest
eligibility. `spend_hit_die` commits one die and returns its roll, Constitution
modifier, actual healing and remaining dice. These resource operations do not
independently authorize resting or spending at any time. The campaign rest and
spending session is #191. `recover` remains the Long Rest resource operation.

The SRD module owns arithmetic, capacities, validation and codecs. Core and Godot
consume typed queries and preserve the opaque continuation, without interpreting
SRD resource strings. The C++20/RAII and Godot GDExtension boundaries are unchanged.

## Persistence

Rules **0.6.10** adds **SRD4** vital continuation when Hit Dice are spent. It stores
existing slots, Second Wind and death-save state, the remaining die count, and an
FX1 effects record. With all dice available, the writer retains compact
SRD1/SRD2/SRD3. Those formats mean unspent dice because earlier modules could not
spend them.

**OGCOMBAT 9** adds each actor's remaining dice. Previous supported modules through
0.6.9 migrate; authored combat definitions without a character recipe receive
zero, without inventing monster Hit Dice mechanics. Campaign format **9** and
PC7 are unchanged: the campaign already stores opaque vital continuations, and
recipes derive capacity from class and level.

Spent dice survive healing, effects, training completion, advancement, combat
turns/checkpoints and campaign handoff. Migration preserves existing wounds,
resources, training, initiative, RNG and time. Earlier facing/grip/overlap
migrations retain their established behavior.

## Verification

`opengold_rest_resource_tests` covers all twelve starting classes, fixed RNG
results, negative Constitution, HP caps, Dwarven Toughness, recharge limits,
caster slots, invalid/dead/unconscious state and rejection atomicity. It checks
campaign round trips, training review, advancement, healing, effects, combat
handoff and malformed checkpoint counts. Frozen 0.6.9 files retain an active
effect, spent slots/Second Wind, equipment and a pending opportunity attack. Its
continuation matches the previous writer apart from format/identity and the new
unspent dice field. See [fixture provenance](../tests/fixtures/README.md).

Existing native and Godot regressions remain required. Rendered rest controls and
their campaign acceptance are #192, not part of these rules tests.

Delivery validation: eleven relevant native suites, six Godot CTest entries and
the existing game/demo party-and-recovery integration routes pass. The demo's
old inn timing assertion was aligned with the game's persisted-rest-timestamp
check so travel time is included. Both GDExtensions rebuild, and localization
and diff checks pass. The first headless game route reported five ObjectDB
instances at shutdown; a verbose rerun passed without that warning. No shutdown
leak fix is claimed by this increment.
