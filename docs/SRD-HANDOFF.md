# SRD handoff

Updated 2026-09-24. **Goal active:** close all `SRD_improvements` issues, with
all twelve classes through level 4, then level 20 and multiclassing. Work one
bounded issue at a time using [SRD-WORKFLOW.md](SRD-WORKFLOW.md).
[Index](https://github.com/stdarg/OpenGold/issues/186), [plan](SRD-IMPLEMENTATION.md),
[coverage](SRD-COVERAGE.md). Query GitHub for counts; old snapshots are stale.

## Current increment

Branch `main`. Completed [Cunning Action #218](https://github.com/stdarg/OpenGold/issues/218):
Rogue ordinary advancement to level two, Bonus Action Dash/Disengage and approved
Q24 row below existing combat buttons. Rules 0.6.35 / PC24; campaign 11 unchanged,
Cunning combat uses existing format 15 Dash counts. Actual 0.6.34 prior-writer
fixtures at `a2aed46` preserve all backgrounds, wounds and spent action budgets.
See [CUNNING-ACTION.md](CUNNING-ACTION.md). Hide remains #219; parent #113 and
Rogue levels three/four #116 remain open. Starting training #213–#217 are complete.

Unarmored Defense was already implemented for Barbarian/Monk in character AC
calculation and covered by party tests; no duplicate changes were needed.
Q24 approved the labeled dropdown and Use Bonus Action button, keyboard access,
and disabled unavailable states. No further UI approval needed for this increment.

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

#218 verification: all 41 native/tool checks pass across the full regression
and targeted Archery/Cunning reruns. Archery's old “Rogues never advance” fixture
now verifies level two is supported and later levels stay unavailable. All 17
Godot runtime checks plus eight native prerequisites pass. Main/demo builds and
816-message localization pass. English/Spanish controls rendered and inspected at
1120×800 and 1920×1080: `/tmp/opengold-cunning-renders`. Logs:
`/tmp/opengold-cunning-regression.log`, `/tmp/opengold-cunning-final-native.log`,
`/tmp/opengold-cunning-godot.log`, `/tmp/opengold-cunning-render.log`.
The final fixture filename correction also passed `opengold_training_tests`.

**Next: inspect Sneak Attack #112 and split its timing/optional damage decisions
into a bounded child if needed.** Verify prerequisites #29/#24 and reuse existing
weapon-hit/damage decision infrastructure. Consult cached SRD text and confirm
any new control behavior before coding. Do not infer full Rogue completion from
level-two support. Pending #80/#208/#209 questions remain above.
