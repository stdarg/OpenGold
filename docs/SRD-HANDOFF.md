# SRD handoff

Updated 2026-09-24. **Goal active:** close all `SRD_improvements` issues, with
all twelve classes through level 4, then level 20 and multiclassing. Work one
bounded issue at a time using [SRD-WORKFLOW.md](SRD-WORKFLOW.md).
[Index](https://github.com/stdarg/OpenGold/issues/186), [plan](SRD-IMPLEMENTATION.md),
[coverage](SRD-COVERAGE.md). Query GitHub for counts; old snapshots are stale.

## Current increment

Branch `main`. #213 completes starting class skill choices for all twelve classes
through the existing Training groups and sourced sheet display. Exact SRD lists
include Fighter Persuasion and Wizard Nature; Bard/Ranger choose three, Rogue
four, others two. Presets fill choices deterministically. Old saves retain missing
choices as pending, while Rogue keeps its prior selections. See
[class skills](CLASS-SKILLS.md). This does not close full class packages or #29/#52.

Rules 0.6.30 / PC19 / FX2; campaign 11 / combat 13–15 unchanged. Actual 0.6.29
fixtures from `8da4565` preserve twelve classes, recorded training, supported
advancement, wounds and spent resources. New class grants reject under old
identities. Reused checkbox callbacks are rebound to the current class source;
controls retain focus and catalog order. Compatible skill choices transfer on
class changes through rules-owned continuity metadata; all 144 class pairs are
verified without putting SRD class names in Core. Main/demo share that implementation.
Entry points: `training.*`, `srd5.cpp`, `training_control.h`, training native/UI
tests and the actual fixtures. Full-party Wizard fixtures clear Fighter training
when temporarily changing class for the untrained-shield test.

**Next: resume #80 if Q23 is answered; otherwise continue independent class
training or passive-trait work.** Q23 remains pending: automatic Great Weapon
Fighting replacement or an optional choice per hit. Do not enable either behavior
without the answer. The [damage-roll foundation](GREAT-WEAPON-FIGHTING.md) is in
`8da4565`; the feat is not selectable/applied. Other styles, Fighter level-up
replacement and mastery remain #85. Q22 approves only the starting style selector.

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

#213 verification is recorded in CLASS-SKILLS.md: 41 native/tool regression checks pass,
including all 144 class-to-class preservation cases. All 16 Godot runtime checks and seven native prerequisites
pass. All-class Training, Wizard/Cleric creation, main/demo creation and full party
flows pass. Main/demo builds and 777-message localization validate. English/Spanish
Bard/Wizard lists inspected at both sizes: `/tmp/opengold-class-skills-renders`.
No live build/test processes remain. Prior feature evidence stays in its documents.
