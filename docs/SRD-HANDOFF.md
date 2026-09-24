# SRD handoff

Updated 2026-09-24. **Goal active:** close all `SRD_improvements` issues, with
all twelve classes through level 4, then level 20 and multiclassing. Work one
bounded issue at a time using [SRD-WORKFLOW.md](SRD-WORKFLOW.md).
[Index](https://github.com/stdarg/OpenGold/issues/186), [plan](SRD-IMPLEMENTATION.md),
[coverage](SRD-COVERAGE.md). Query GitHub for counts; old snapshots are stale.

## Current increment

Branch `main`. Completed [Unconscious enemy transit #222](https://github.com/stdarg/OpenGold/issues/222),
a bounded child of #44. Rules 0.6.36 corrects passing through living zero-HP
enemies with one Difficult Terrain surcharge, preserving occupied endpoints,
reaction interruptions and involuntary overlap through recovery/exit. PC24,
campaign 11 and combat formats 13–15 are unchanged. Prior 0.6.35 fixtures at
`ff297ef` preserve exact state and Dash continuation. See
[UNCONSCIOUS-TRANSIT.md](UNCONSCIOUS-TRANSIT.md). #44 remains open for size/Tiny
rules and Prone; #45 covers footprints and #35 other condition sources.

[Sneak Attack #220](https://github.com/stdarg/OpenGold/issues/220) remains pending
Q25; its tested eligibility/progression helper and real 0.6.35 baseline fixtures
are in `ff297ef`, not wired into combat. See [SNEAK-ATTACK.md](SNEAK-ATTACK.md).
**Q25:** centered eligible-hit dialog with target/extra dice, Use Sneak Attack or
Keep hit; save Sneak Attack. Use spends this turn's use; Savage Attacker follows
and rerolls weapon dice only. Other actions wait; Action/Reaction stays spent;
keyboard access, no combat-saving controls. Do not implement the dependent
control/decision ordering until answered. Continue independent backlog work.

Parent #112 remains open; level-three/four integration is #221 after #116.
Cunning Action #218 is in `3846c21`: Rogue level two and Bonus Dash/Disengage.
Hide #219, parent #113 and Rogue integration #116 remain open. Starting training
#213–#217 are complete. Barbarian/Monk Unarmored Defense was already implemented.

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

#222 verification: all 41 native/tool checks, all 17 Godot runtime checks plus
eight native prerequisites, main/demo builds and 816-message localization pass.
The extended existing opportunity UI test verifies actual clicks through the
loaded corridor, rejected occupied endpoints, twenty feet spent and the unspent
Action. Rules checks also cover reaction interruption, healing, death/natural
recovery and exit. Logs: `/tmp/opengold-transit-regression.log`,
`/tmp/opengold-transit-ui.log`, `/tmp/opengold-transit-recovery.log`.
No layout or text changes; existing movement controls are reused.

#220 preparation: focused damage/training checks passed; no live feature claim.
Next: resume #220 if Q25 arrives; otherwise select an independently actionable
source-backed issue. Preserve pending #80/#208/#209/#189 questions above rather
than re-asking. Do not close parent trackers from narrow child evidence.
