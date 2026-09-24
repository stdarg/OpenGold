# SRD handoff

Updated 2026-09-24. **Goal active:** close all `SRD_improvements` issues, with
all twelve classes through level 4, then level 20 and multiclassing. Work one
bounded issue at a time using [SRD-WORKFLOW.md](SRD-WORKFLOW.md).
[Index](https://github.com/stdarg/OpenGold/issues/186), [plan](SRD-IMPLEMENTATION.md),
[coverage](SRD-COVERAGE.md). Query GitHub for counts; old snapshots are stale.

## Current increment

Branch `main`. Active [Sneak Attack #220](https://github.com/stdarg/OpenGold/issues/220),
child of #112. Source eligibility/progression helper and actual 0.6.35 baseline
fixtures are prepared; see [SNEAK-ATTACK.md](SNEAK-ATTACK.md). No live behavior or
version changes: rules 0.6.35 / PC24, campaign 11 / combat 13–15.

**Q25 pending:** centered eligible-hit dialog with target/extra dice, Use Sneak
Attack or Keep hit; save Sneak Attack. Use spends this turn's use; Savage Attacker
follows and rerolls weapon dice only. All other actions wait; Action/Reaction stays
spent; keyboard access, no combat-saving controls. Do not implement the dependent
control/decision ordering until answered. Continue independently actionable backlog
work while waiting, preserving #220's complete acceptance.

Parent #112 remains open. Level-three/four integration is #221 after ordinary
advancement #116. Existing grants infrastructure is sufficient despite wider #29
remaining open; prerequisite #24 is closed. Read #220 and its feature doc before
integration. Distinguish Ranged weapons from thrown Melee attacks, combine attack
damage before resistance/flooring, and reset use on every combatant turn.

Cunning Action #218 is delivered in `3846c21`: ordinary Rogue level two, Bonus
Dash/Disengage, approved Q24 row. Hide #219, parent #113 and Rogue integration #116
remain open. Starting training #213–#217 are complete. Unarmored Defense already
exists for Barbarian/Monk; no duplicate implementation needed.

Q23 remains pending for #80: automatic Great Weapon Fighting replacement or an
optional choice per hit. Do not enable either without the answer. The tested
[damage-roll foundation](GREAT-WEAPON-FIGHTING.md) is in `8da4565`; the feat is not
selectable/applied. Q22's starting Fighting Style selector is already delivered.

## Next work and pending questions

[Silence combat/area integration #208](https://github.com/stdarg/OpenGold/issues/208)
and [campaign/ritual integration #209](https://github.com/stdarg/OpenGold/issues/209)
remain dependent on unanswered questions 19–21. Do not re-ask or implement their
dependent choices without answers. These requests cite AGENTS.md for controls
and explicit review for geometry:

- Q19: flat grid; square center within 120 feet, circular 20-foot radius, whole
  occupied square determines full containment, walls block spread; preview
  distinguishes partial squares and fully contained creatures. No height model.
- Q20: prepared “Silence — level 2” in existing Spell dropdown; Cast enters area
  preview, arrows move it, Enter/click commits, Escape cancels free. New row below
  Dash shows End concentration, spell and duration. Release costs no action and
  is available for the selected owner outside their turn.
- Q21: Silence in current level 3–4 Cleric preparation choices, retaining current
  limits/confirmation; explicit choice only, no change to saved preparations.

Concentration lifecycle #207 and isolated CN1 codec are delivered; CN1 is not
embedded in live saves. See [CONCENTRATION.md](CONCENTRATION.md).
#205 Ray of Frost remains partial for other grant routes and #39 speech blockers;
see [RAY-OF-FROST.md](RAY-OF-FROST.md). Mundane gagging remains separate.
Continue independently actionable backlog work while these questions are pending.

## Decisions to preserve

- Saves only at camp/inn; no player combat-save controls. Internal continuation
  fixtures/checkpoints are permitted.
- Missing old training selections remain pending. Review Training #189 has
  unanswered layout question 11; retrieve its wording if needed. Presets must
  have pre-generated training selections.
- Approved creation steps: Training, then class-driven Spell Choices, then Name.
  Back keeps valid selections; unsupported catalog choices remain pending.
- Q18 approved and delivered: shared Spell dropdown and Cast in the existing
  combat row to the right of Adrenaline Rush; known cantrips, legal target preview,
  keyboard access, A/Space cycle and disabled unavailable casting. Reuse approval.
- Grip, HP source tooltips/colors, Temp HP replacement, Adrenaline Rush, Savage
  Attacker two-stage choice and Action Surge controls are approved; see feature docs.
- Play `/System/Library/Sounds/Glass.aiff` with `/usr/bin/afplay` before numbered
  questions. Batch related questions and reuse approvals. Fixed data displayed
  by existing controls does not require another layout approval.

## Environment and verification

Bash/macOS. `build/mac-check` is the main build, `build/sprite-demo` separate.
Godot: `/Applications/Godot_mono.app/Contents/MacOS/Godot`; project:
`src/OpenGoldBox/godot`; local original assets: `/Users/edmond/POOLRAD`.
Run Godot tests serially (shared checkpoint paths). Prepare the project once after
native/localization changes; see workflow for fixture exclusion and test commands.
Finished creation training appears in `Description`, not `ModifiersModal`.
Refresh locale-dependent creator text with Back/Next after changing locale.

#220 foundation verification: `opengold_damage_tests` and
`opengold_training_tests` pass. Independent catalog/roll circumstances and full
source dice progression are checked; actual 0.6.35 campaign and both Savage hit
continuations preserve exact state. Log: `/tmp/opengold-sneak-foundation.log`.
No live feature/UI change; no broader regression claim for unimplemented behavior.

Previous #218 evidence: 41 native/tool checks, 17 Godot runtime checks plus eight
native prerequisites, main/demo builds and 816-message localization pass.
Bilingual renders at both sizes: `/tmp/opengold-cunning-renders`. Logs begin
`/tmp/opengold-cunning-`. Do not repeat unrelated checks without new changes.
