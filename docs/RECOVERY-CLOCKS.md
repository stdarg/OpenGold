# Death saves and recovery clocks

[F04a #194](https://github.com/stdarg/OpenGold/issues/194) adds rules-owned recovery
clocks and shared vitality transitions. Parent [F04 #31](https://github.com/stdarg/OpenGold/issues/31)
remains open until [campaign scheduling #195](https://github.com/stdarg/OpenGold/issues/195)
continues them outside combat. Help/Medicine controls remain [#32](https://github.com/stdarg/OpenGold/issues/32).
No new controls or UI layout are introduced here. The existing all-unconscious
party defeat policy is unchanged.

Authority: [SRD 5.2.1](https://media.dndbeyond.com/compendium-images/srd/5.2/SRD_CC_v5.2.1.pdf),
pp. 17–18, Death Saving Throws and Stabilizing a Character.

## Rules and combat

`life_cycle.h/.cpp` owns value-based vitality transitions and clock validation.
Actors retain this state by value; there is no resource ownership through raw
pointers and no Godot dependency.

- An unstable living creature at zero HP rolls at turn entry, including the
  initial initiative slot. Three successes make it Stable and clear both counters;
  three failures kill it. A natural 1 adds two failures; a natural 20 restores
  one HP and permits that turn without replenishing resources.
- Becoming Stable rolls its 1d4-hour recovery delay once. Repeated stabilization
  preserves the existing delay. Combat elapsed time reduces the countdown and
  restores exactly one HP at its deadline. Ordinary turns remain governed by
  initiative; waking between slots does not grant an immediate extra turn.
- Any positive damage at zero HP ends Stable and cancels its recovery deadline.
  It adds one death-save failure, or two for a critical hit. Massive damage and
  normal enemy death-at-zero policy remain intact. The existing commands do not
  newly offer unconscious targets; their broader condition/targeting rules remain
  separate work.
- Positive healing clears mortality timers and both counters. Zero healing or
  zero damage does not change the recovery state. Script HP changes now use the
  rules adapter, preserving spent pools while clearing obsolete timers.
- A new encounter schedules death saves on its initiative slots. A restored
  encounter retains exact saved timing and adds no roll. After a death save,
  the next-turn countdown starts at six seconds and decrements with combat time.
  Campaign handoff preserves that remainder; advancing it outside combat is #195.

The campaign's existing effect scheduler currently preserves these clocks but
does not yet advance death saves or Stable recovery outside combat. #195 must
merge the recovery and effect events chronologically, process reserves, and prove
that time subdivisions and repeated combat synchronization cannot alter RNG or
apply elapsed time twice. This child is not completion of F04.

## Persistence and migration

Rules **0.6.11** writes **OGCOMBAT 10**. Each actor row adds the remaining death-save
and Stable-recovery milliseconds after its Hit Dice count. **SRD5** vital
continuation carries both clocks alongside existing pools, counters, Hit Dice
and FX1 effects for living zero-HP characters. Healthy/dead actors retain the
existing compact resource formats. Campaign format **10** and PC7 are unchanged.

Clocks must agree with vitality: healthy/dead creatures have none, Stable
creatures have no death-save timer, and unstable creatures have no Stable timer.
Death-save intervals cannot exceed six seconds and a Stable countdown cannot
exceed four hours. Malformed clocks reject before replacing live state.

Earlier supported campaign formats and combat modules through **0.6.10** retain
wounds, counters, spent resources, effects, equipment, RNG and pending movement.
Their timer history is unknown and never backdated. Legacy unstable campaign
state starts with a full six-second interval when the continuation is adopted;
legacy combat state uses its next initiative slot. Legacy Stable state records a
pending duration roll (zero remaining milliseconds) until elapsed-time processing
first begins. Loading, viewing and saving alone never roll it or heal a character.
New stabilization rolls immediately; it never uses the deferred legacy path.

Frozen files from commit `1adfc63`, module 0.6.10, cover Stable status with a spent
Hit Die, an unstable caster with Blinded/spent slots, a dead member, an unconscious
reserve, and a pending opportunity decision. See [fixture provenance](../tests/fixtures/README.md).

## Verification

`opengold_recovery_clock_tests` checks independent fixed rolls and RNG values,
third-success stabilization, one-time duration selection, exact recovery deadlines,
healing/damage/death transitions, overflow and malformed timer rejection. Combat
and campaign checkpoints retain timers and resource expenditure; training review,
advancement and temple healing keep or cancel them as appropriate. Frozen old
checkpoints preserve the previous writer's pending-reaction continuation, adding
only the new known/deferred timing fields. Existing death-save turn-entry,
shared-space, grant, training, rest, party, save and effect tests remain required.

Delivery checks passed: twelve targeted native suites (including fuzz smoke),
the demo and game `--party-check` routes, six Godot integration checks and
localization validation for all 559 English/Spanish messages. Both extensions
were rebuilt. Scope review confirms the existing C++20/RAII and Godot boundaries;
this increment makes no independent UI layout or control choices.
