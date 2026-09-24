# SRD handoff

Updated 2026-09-24. **Goal active:** close all `SRD_improvements` issues, with
all twelve classes through level 4, then level 20 and multiclassing. Work one
bounded issue at a time using [SRD-WORKFLOW.md](SRD-WORKFLOW.md).
[Index](https://github.com/stdarg/OpenGold/issues/186), [plan](SRD-IMPLEMENTATION.md),
[coverage](SRD-COVERAGE.md). Query GitHub for counts; old snapshots are stale.

## Current increment

Branch `main`. Approved question 22 is implemented under #85: Fighter Training
has a required Archery/Defense dropdown above languages, preserved on Back and
keyboard accessible. Presets receive a choice; old saves retain a pending choice.
See [Fighter styles](FIGHTER-STYLES.md). #85 remains open for other styles,
replacement on Fighter level-up and weapon mastery. #78/#79 retain remaining
class routes. #212 is closed; its parents #61/#64 retain package work.

Rules 0.6.29 / PC18 / FX2; combat 13–15 and campaign 11 unchanged. Starting
style grants use `class:fighter:fighting_style` at level one, separate from level
four. Nonrepeatability spans both sources. Actual 0.6.28 campaign/combat fixtures
preserve all prior choices, wounds and expenditure without assigning a style.
Old profiles and identities reject the new source. Entry points: `training.*`,
`feature_grants.*`, `srd5.cpp`, `training_control.h`, `training_tests.cpp` and
`training_view_tests.gd`. Main/demo creation controls use the same group metadata.

**Next: continue #85 with remaining style/replacement/mastery work.** Review its
scope and split distinct state machines before implementation. Replacement is
not old-save completion: the existing pending-training API preserves chosen
selections. Any new level-up control layout still needs a numbered question;
Q22 only approves the starting Training dropdown. Other Fighting Style feats
retain their existing issue tracking. Do not claim all class choices complete.

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

Starting-style verification is recorded in FIGHTER-STYLES.md. Native regression
covered 41 checks; six old training-completion fixtures now include the required
style and pass their focused rerun. All 16 Godot runtime checks and their native
prerequisites pass after rebuilding those fixtures. Main/demo extension builds,
755-message localization and keyboard selection checks pass. English/Spanish
Training layouts were inspected at 1120×800 and 1920×1080 under
`/tmp/opengold-styles-renders`. Full party-path verification also exercises the
new choice; its untrained-shield fixture clears the Fighter style when temporarily
constructing a Wizard. Native checks use translated bonus-source assertions. No live build/test processes remain.
