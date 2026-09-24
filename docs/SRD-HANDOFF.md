# SRD handoff

Updated 2026-09-24. **Goal active:** close all `SRD_improvements` issues, with
all twelve classes through level 4, then level 20 and multiclassing. Work one
bounded issue at a time using [SRD-WORKFLOW.md](SRD-WORKFLOW.md).
[Index](https://github.com/stdarg/OpenGold/issues/186), [plan](SRD-IMPLEMENTATION.md),
[coverage](SRD-COVERAGE.md). Query GitHub for counts; old snapshots are stale.

## Current increment

Branch `main`. #78 now has Archery through the existing Fighter level-four feat
selector: prerequisite/provenance, +2 for Ranged weapons, persistence, translated
selection and sheet name. See [Archery](ARCHERY.md). #78 remains open until the
starting/other class grant routes are integrated. #212 fixed Acolyte/Soldier
training is closed (`131942d`); its parents #61/#64 retain remaining package work.

Rules 0.6.28 / PC17 / FX2; combat 13–15 and campaign 11 unchanged. The actual
0.6.27 campaign/combat fixtures retain their entire bodies except module identity.
Old profiles and campaign identities reject newly introduced Archery grants.
Entry points: `feature_grants.{h,cpp}`, `srd5.cpp`, `level_up_view.cpp`,
`character_sheet_view.cpp`, `tests/archery_tests.cpp` and `tests/training_tests.cpp`.

**Next: Fighter starting Fighting Style under #85. Question 22 is approved:**
a labeled dropdown in the existing Training step, above language choices;
initially Archery and Defense; one required selection before Next; Back preserves
it; keyboard access; presets pre-generated; old saves retain a pending choice.
Other styles remain tracked. Do not re-ask this layout question. Selection and
replacement/provenance need rules integration before claiming #85 complete;
weapon mastery also remains in that parent. Review existing Training controls,
CharacterDraft save schema, grant validation and old-choice migration first.

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

#78 validation is recorded in ARCHERY.md. Current native regression: 41 passing
checks; 16 Godot runtime checks plus seven fixture prerequisites pass. Actual
advancement confirmation/reload passes. Main/demo builds and 750-message
localization validate. Capture artifacts: `/tmp/opengold-archery-renders`.
English/Spanish dialogs were inspected at 1120×800 and 1920×1080. Final focused
Archery checks pass after strengthened atomicity/version assertions. No live
build/test processes remain. Native Godot advancement checks use translated
bonus-source assertions, allowing the full confirmation sequence in Spanish.
