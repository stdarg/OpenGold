# SRD handoff

Updated 2026-09-24. **Goal active:** close all `SRD_improvements` issues, with
all twelve classes through level 4, then level 20 and multiclassing. Work one
bounded issue at a time using [SRD-WORKFLOW.md](SRD-WORKFLOW.md).
[Index](https://github.com/stdarg/OpenGold/issues/186), [plan](SRD-IMPLEMENTATION.md),
[coverage](SRD-COVERAGE.md). Query GitHub for counts; old snapshots are stale.

## Current increment

Branch `main`. #80 Great Weapon Fighting has a shared damage-roll foundation;
see [evidence and remaining integration](GREAT-WEAPON-FIGHTING.md). Combat uses
the normal rule; the feat is not yet selectable/applied. Independent tests prove
per-die replacement without rerolls, critical dice with one flat modifier and
successive Savage rolls. Actual 0.6.29 files from `88c2649` reproduce a critical
Greatsword/Savage sequence exactly. No format/module version changed.

**Q23 is pending:** automatic beneficial replacement, or an optional choice on
each eligible hit. Do not enable either combat behavior until answered. Meanwhile
other independently actionable backlog work is authorized. For full #80, add
eligibility/grants, both starting and advancement selectors, reactions/current
continuation validation, spell/unarmed/thrown exclusions and class routes.
`damage_roll.h`, `srd5.cpp` and `damage_tests.cpp` are the foundation entry points.

Fighter starting Archery/Defense (#85, approved Q22) is delivered in `88c2649`:
required dropdown above languages, keyboard access, Back preservation, generated
presets and old pending choices. Rules 0.6.29 / PC18 / FX2; campaign 11 / combat
13–15. See [Fighter styles](FIGHTER-STYLES.md). #85 remains open for other styles,
replacement on Fighter level-up and mastery; #78/#79 retain class routes.
Replacement is not pending-training completion: that API preserves chosen
selections. Any new level-up control layout requires a numbered question.

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

Starting-style verification remains in FIGHTER-STYLES.md. For the damage-roll
foundation, all 41 native/tool checks pass; main/demo builds and the combat/Savage
Godot checks (plus native prerequisite) pass. No new UI or localization changes.
Logs: `/tmp/opengold-gwf-regression.log`, `/tmp/opengold-gwf-godot.log` and
`/tmp/opengold-gwf-demo.log`. No live build/test processes remain.
