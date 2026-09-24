# SRD handoff

Updated 2026-09-24. **Goal active:** close all `SRD_improvements` issues, with
all twelve classes through level 4, then level 20 and multiclassing. Work one
bounded issue at a time using [SRD-WORKFLOW.md](SRD-WORKFLOW.md).
[Index](https://github.com/stdarg/OpenGold/issues/186), [plan](SRD-IMPLEMENTATION.md),
[coverage](SRD-COVERAGE.md). Query GitHub for counts; old snapshots are stale.

## Current increment

Branch `main`. Completed increment: [#224](https://github.com/stdarg/OpenGold/issues/224),
level-one Warlock Eldritch Blast creature casting, under #160/#204. **Q26 approved:**
reuse the existing Spell Choices pattern, two owed cantrips, second pending until
supported, pre-generated presets, old selections retained, shared Spell/Cast.
Rules 0.6.37 / PC25 implements explicit Pact Magic cantrip grants and Charisma
attacks. [Scope and evidence](ELDRITCH-BLAST.md). Objects are a separate prerequisite
[#225](https://github.com/stdarg/OpenGold/issues/225); levels 2–4, full Pact Magic,
Tome/invocations and speech blockers remain open. Do not close #204 from this child.
Shocking Grasp [#223](https://github.com/stdarg/OpenGold/issues/223) is queued;
no implementation landed. It can reuse approved shared cantrip controls.

Previous increment: [Unconscious transit #222](https://github.com/stdarg/OpenGold/issues/222)
is complete in `15fcda5`; see [scope](UNCONSCIOUS-TRANSIT.md). Its existing saved
state and Dash continuation remain preserved.

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

#224 verification: all 42 native/tool checks, final focused Eldritch checks,
all 18 headless Godot checks plus nine native prerequisites, and asset-backed
Warlock creator checks passed. Main/demo builds and 820-message localization
pass. English/Spanish creation/combat renders cover 1120×800 and 1920×1080.
Localized Cast follows the dropdown's actual width to avoid overlap.
Logs: `/tmp/eb-regression.log`, `/tmp/eb-final-checks.log`, `/tmp/eb-creator.log`,
`/tmp/eb-combat-render.log`; renders `/tmp/opengold-eldritch-renders`.
Creator checks require `OPENGOLD_GAME_DIR=/Users/edmond/POOLRAD`; they are not
registered in the asset-free headless suite. See the feature doc for scope.

Next: resume #220 if Q25 arrives; otherwise Shocking Grasp #223 can reuse the
approved Spell/Cast controls. Preserve pending #80/#208/#209/#189 questions.
Do not close parent trackers from narrow child evidence. No live processes remain.
