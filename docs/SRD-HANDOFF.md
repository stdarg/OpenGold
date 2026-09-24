# SRD handoff

Updated 2026-09-24. **Pause requested after #228:** close all `SRD_improvements` issues, with
all twelve classes through level 4, then level 20 and multiclassing. Work one
bounded issue at a time using [SRD-WORKFLOW.md](SRD-WORKFLOW.md).
[Index](https://github.com/stdarg/OpenGold/issues/186), [plan](SRD-IMPLEMENTATION.md),
[coverage](SRD-COVERAGE.md). Query GitHub for counts; old snapshots are stale.

## Current increment

Branch `main`. [Sorcerer cantrip access #228](https://github.com/stdarg/OpenGold/issues/228)
adds the four currently implemented eligible cantrips at level one, explicit
`class:sorcerer:spellcasting` grants and Charisma attacks. **Q27 approved** the
existing Spell Choices and Spell/Cast patterns, preset choices and pending old
selections. [Scope/evidence](SORCERER-CANTRIPS.md). Rules 0.6.40 / PC28;
campaign 11, combat 13–15 and FX1–3 stay unchanged. Actual 0.6.39 writer fixtures
at `6988432` retain old choices, spent resources and exact turn continuation.

**User requested pausing after #228 for a progress discussion.** Implementation
and checks are complete; commit/push and issue closure are the final delivery steps
before the goal is paused. Do not start another issue until the user resumes. Sorcerer #132 remains open for the complete catalog,
leveled spells/slots, Innate Sorcery and later levels/replacements. Fire Bolt's
existing enemy-only targeting gap is recorded on #166; source access does not
close full spell conformance.

Previous: Warlock Poison Spray #227 in `6988432`, Wizard Shocking Grasp #226 in
`3b7e943`, Warlock Eldritch Blast #224 in `310319e`. Feature docs retain remaining
source/level/object work. Q26 remains approved for Warlock creation/casting.

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

#228 verification: all 44 native/tool checks and 20 headless Godot checks plus
eleven native prerequisites pass. Main/demo builds and 827-message localization
pass. Asset-backed creator and graphical combat checks pass; English/Spanish
renders cover 1120×800 and 1920×1080. Logs: `/tmp/sorcerer-regression.log`,
`/tmp/sorcerer-creator.log`, `/tmp/sorcerer-combat-render.log`,
`/tmp/sorcerer-demo.log`; renders: `/tmp/opengold-sorcerer-renders`.
Creator checks require `OPENGOLD_GAME_DIR=/Users/edmond/POOLRAD`; they are not
registered in the asset-free headless suite. No live processes remain.

After the user resumes, #220 can proceed if Q25 arrives; otherwise select
independently actionable backlog work.
Preserve pending #80/#208/#209/#189 questions rather than re-asking them.
Do not close parent trackers from narrow child evidence.
