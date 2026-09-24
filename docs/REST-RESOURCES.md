# Rest resources

[F03a #190](https://github.com/stdarg/OpenGold/issues/190) implements Hit Dice and
supported rest recharge in the rules layer. [F03b #191](https://github.com/stdarg/OpenGold/issues/191)
adds individual campaign eligibility and a persistent spending session. Parent
[F03 #30](https://github.com/stdarg/OpenGold/issues/30) remains open until the
[controls #192](https://github.com/stdarg/OpenGold/issues/192) and
[interruptions/resumption #193](https://github.com/stdarg/OpenGold/issues/193) are delivered.
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
species-specific rest features remain their named issues. General interruption
scheduling and Long Rest resumption are not implemented; the campaign boundaries
below are explicit. Player-facing spending controls remain #192.

## Architecture and API

`RulesModule::recovery_info` reads die size, remaining/maximum dice, vitality
eligibility and named supported resource pools. Each pool reports its capacity,
remaining uses and Short Rest recharge amount. `can_rest` checks vitality only;
activity and cooldown checks belong to the campaign.

`recover_short_rest` applies recharge after the caller establishes completed-rest
eligibility. `spend_hit_die` commits one die and returns its roll, Constitution
modifier, actual healing and remaining dice. These resource operations do not
independently authorize resting or spending at any time. `recover` remains the
Long Rest resource operation.

The SRD module owns arithmetic, capacities, validation and codecs. Core and Godot
consume typed queries and preserve the opaque continuation, without interpreting
SRD resource strings. The C++20/RAII and Godot GDExtension boundaries are unchanged.

## Campaign transactions

`CampaignParty::rest_info(kind)` returns each active member's recovery resources,
eligibility denial and precise remaining cooldown. Dead/zero-HP members are
ineligible. Reserves are absent. Eligibility is captured at the start: becoming
eligible during another member's rest does not grant benefits retrospectively.

`rest(kind)` requires the host to establish a safe location and interruption
profile. It stages elapsed time, effect recovery rolls and individual benefits
in one owned state copy. All roster effects advance once; only eligible active
members receive recharge and Long Rest completion timestamps. No eligible member
means no elapsed time or other changes. The existing `rest()` wrapper selects
Long Rest. Removal/rejoin and save/load preserve each member's cooldown.

A successful Short Rest creates a `ShortRestSession` at the completed hour, with
eligible member IDs and a session/revision ticket. `spend_hit_die(ticket, member)`
commits one roll, healing and resource expenditure, then increments the revision.
Repeated callbacks with the previous ticket reject. The player can decide again
after every roll. `finish_short_rest(ticket)` closes the window, including with
zero dice spent, without reverting healing, dice, recharge or time.

Ordinary party mutations and town events are barred while spending is pending;
selection and save/load are allowed. Positive elapsed time or a successful combat
handoff closes the window. Failed time updates or combat initialization preserve
it. Further rests require finishing the current window. Trusted native checkpoint
restoration remains available for load and campaign event rollback; the spending
flow never uses it for cancellation.

`RolfTourSession::camp(kind)` runs original ECL entry 2 for both kinds. Forbidden
camping grants nothing. The supported guaranteed city-watch interruption advances
five minutes and runs entry 3, granting no recovery or spending session. Other
probabilistic profiles remain explicitly unsupported and roll back. Safe camp
completes the requested kind. Existing Camp [C] still requests Long Rest until
#192 adds reviewed controls; the original paid inn PROGRAM 9 still requests Long
Rest. Failed inn continuations restore the entire event, including payment,
resources, cooldowns, time and RNG. See [recovery mappings](RECOVERY.md).

The native campaign activity now tracks resting, sleep, light activity and physical
exertion separately. Existing safe-camp/inn calls consume this engine atomically.
Revisioned requests support interruption, resumption and abandonment; rejected or
stale requests preserve time, effects, resources and RNG. Long Rest requires six
hours of sleep and no more than two light hours, with an extra resting hour per
interruption. At least one fresh uninterrupted resting hour earns Short Rest
benefits (Q32); previously credited segments cannot qualify again. Completing or
abandoning a rest never reverses committed Hit Dice or elapsed time.

Activity checkpoints retain eligible members and progress across save/load and
explicitly interrupted combat handoff. Physical exertion consumes campaign time
without increasing resting time; reaching one hour interrupts a Long Rest.
Initiative, non-cantrip casting and damage have explicit interruption inputs.
Hosts must resolve earned Hit Dice choices before combat/time advancement, check
camp permission before resuming and prevent unrelated exploration while a resume
decision is pending. Automatic event connections, sleeping actors' Unconscious
behavior and reviewed Godot controls are still pending in #192/#193. The existing
five-minute city-watch route remains unchanged; unknown probabilistic profiles
never silently substitute an uninterrupted rest.

## Persistence

Rules **0.6.10** adds **SRD4** vital continuation when Hit Dice are spent. It stores
existing slots, Second Wind and death-save state, the remaining die count, and an
FX1 effects record. With all dice available, the writer retains compact
SRD1/SRD2/SRD3. Those formats mean unspent dice because earlier modules could not
spend them.

**OGCOMBAT 9** adds each actor's remaining dice. Previous supported modules through
0.6.9 migrate; authored combat definitions without a character recipe receive
zero, without inventing monster Hit Dice mechanics. PC7 is unchanged; recipes
derive capacity from class and level.

Campaign format **10** appends the next rest-session ID and optional completed
Short Rest continuation after the existing party/town payload. It stores the
session/revision ticket, completion time and eligible member IDs. Formats 1–9
load with no pending spending session: their elapsed time never invents a rest
entitlement. Malformed, duplicate, inactive or ineligible members and mismatched
completion times reject. The writer retains opaque spent-resource continuations;
loading a pending session repeats neither the hour nor feature recharge.

Spent dice survive healing, effects, training completion, advancement, combat
turns/checkpoints and campaign handoff. Migration preserves existing wounds,
resources, training, initiative, RNG and time. Earlier facing/grip/overlap
migrations retain their established behavior.

Rules 0.6.11 subsequently adds SRD5 for living zero-HP characters and combat 10
mortality clocks. They retain the same Hit Dice/pool fields; ordinary healing
cancels mortality timing without restoring expenditure. Campaign format 10 is
unchanged. [Recovery clocks](RECOVERY-CLOCKS.md) describes that later increment
and the still-pending outside-combat scheduler.

## Verification

`opengold_rest_resource_tests` covers all twelve starting classes, fixed RNG
results, negative Constitution, HP caps, Dwarven Toughness, recharge limits,
caster slots, invalid/dead/unconscious state and rejection atomicity. It checks
campaign round trips, training review, advancement, healing, effects, combat
handoff and malformed checkpoint counts. Frozen 0.6.9 files retain an active
effect, spent slots/Second Wind, equipment and a pending opportunity attack. Its
continuation matches the previous writer apart from format/identity and the new
unspent dice field. See [fixture provenance](../tests/fixtures/README.md).

`opengold_campaign_rest_tests` covers mixed eligibility/cooldowns, active PC/NPC
and reserve boundaries, exact clock thresholds, one-die decisions, zero spending,
duplicate/expired requests, save/load between rolls, next combat, overflow and
malformed continuation rejection. It compares effect/RNG processing against one
independent elapsed interval and tests the original safe/denied/interrupted/
unsupported camp paths plus inn rollback. Existing save/party regressions retain
old-format migrations and per-member cooldown behavior.

Rendered rest controls and their campaign acceptance remain #192.

F03a delivery validation: eleven relevant native suites, six Godot CTest entries and
the existing game/demo party-and-recovery integration routes pass. The demo's
old inn timing assertion was aligned with the game's persisted-rest-timestamp
check so travel time is included. Both GDExtensions rebuild, and localization
and diff checks pass. The first headless game route reported five ObjectDB
instances at shutdown; a verbose rerun passed without that warning. No shutdown
leak fix is claimed by this increment.

F03b delivery validation: ten relevant native suites, six Godot CTest entries,
and the game/demo party-and-recovery routes pass. Both GDExtensions rebuild;
localization and diff checks pass. The game route again reports the previously
observed five ObjectDB instances at shutdown; no shutdown fix is included here.

Rules 0.6.12 advances mortality alongside effects during rest time. A zero-HP
companion or reserve may wake naturally with one HP or die; eligibility captured
at the start still prevents unearned recharge, Hit Dice or completion timestamps.
See [recovery scheduling](RECOVERY-CLOCKS.md#campaign-time).

Rules 0.6.24 adds Fighter Action Surge at level 2: one use through level 4,
fully recharged by either rest kind. Advancement preserves expenditure. SRD8
stores the spent use alongside existing resources; compact earlier formats
mean the new pool is available. See [Action Surge](ACTION-SURGE.md).

## Rest batch B: verified profile boundary

The campaign adapter now accepts only the documented guaranteed city-watch
profiles (`6DD2=1`, `6DD3=100/101`), alongside already-supported safe/forbidden
profiles. Previously it treated other chance values above 101 as city-watch
interruptions without evidence. A regression covering 102, 200 and 254 failed
before the correction; these profiles now reject without advancing time, changing vitals
or RNG, or granting a spending entitlement. Both rest kinds retain their verified
100/101 behavior. This closes that boundary error only: #30/#192/#193 remain
open for the controls and resumable rest activity described above. Pending
layout/behavior decisions are Q29–31; Q32 approves fresh qualifying rest segments. See the
[decision register](SRD-DECISIONS.md).

## Resumable activity persistence and evidence

Campaign format **12** appends an optional activity record after the existing
rest-spending continuation. It records the session/revision, kind, start clock,
resting/fresh-segment/sleep/light/exertion/extension milliseconds, interruption
state/cause, current work and eligible IDs. Only saves containing an activity use
12; ordinary saves retain 11. Readers retain formats 1–11 and validate activity
structure, elapsed-clock bounds and rules timing before replacing live state.
Rules identity 0.6.40, PC28 and combat formats remain unchanged.

`tests/rest_activity_checks.h`, run by `opengold_campaign_rest_tests`, verifies
fresh-segment recovery, millisecond boundaries, light/sleep and exertion limits,
stale/duplicate/overflow rejection, interrupted combat handoff, abandonment and
correct-checksum malformed records. Frozen actual `aeeb3b7` writer fixtures prove
byte-identical loading and the next Hit Die result/RNG for existing spending
sessions. See `tests/fixtures/README.md` for provenance. These native checks do
not constitute completion of the remaining player workflow.

Final native-activity tree verification: all 44 native/tool checks and all 20
registered Godot checks passed (31 CTest entries including fixtures). Rebuilt
actual game/demo party routes passed original camping, temple, inn and recovery
checks. Scope review confirms C++20 Core/rules boundaries and value-owned state;
no new UI, runtime, save location or combat-saving control was introduced.

### Interrupted encounter rewards

XP and original encounter loot may commit while a Long Rest is interrupted and
its earned Hit Dice choices are resolved. The rest retains elapsed progress and
its extension; a committed reward advances the activity revision. Duplicate
reward IDs remain idempotent, and failed validation/overflow preserves all state.
This exception does not unlock roster, equipment, training or leveling changes.
Running rests and pending Hit Dice choices still reject reward changes. The
post-combat ECL readback may synchronize identical HP/wealth without mutation;
changed script values still reject while activity is retained.

`party_tests.cpp::interrupted_rest_victory` drives a complete real combat to
victory, retains XP/loot, checks duplicate and overflow rollback, saves/reloads,
and resumes through Long Rest completion. Before the fix the victory threw
“Finish or abandon the rest before changing the party.” This closes the native
reward handoff gap; automatic encounter scheduling and effectful ECL script
adapters remain part of #193, alongside the pending reviewed controls.

Reward-handoff final verification passed all 44 native/tool and 20 registered
Godot checks, plus actual game/demo party/recovery routes. Existing formats and
module identity are unchanged; this introduces no control or layout changes.
