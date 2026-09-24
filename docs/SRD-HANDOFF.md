# SRD handoff

Updated 2026-09-24. **Goal active:** close all `SRD_improvements` issues, with
all twelve classes through level 4, then level 20 and multiclassing. Work one
bounded issue at a time using [SRD-WORKFLOW.md](SRD-WORKFLOW.md).
[Index](https://github.com/stdarg/OpenGold/issues/186), [plan](SRD-IMPLEMENTATION.md),
[coverage](SRD-COVERAGE.md). Query GitHub for counts; old snapshots are stale.

## Current increment

Branch `main`. [Bard instruments #214](https://github.com/stdarg/OpenGold/issues/214)
is implemented and verified. Bards choose three of all ten SRD instruments through
Training; presets generate choices, sheets show sources and checks apply the
existing proficiency/Advantage rules. Old choices remain pending. Rules 0.6.31 /
PC20 / FX2; campaign 11 / combat 13–15 unchanged. Real prior-writer fixtures stay
unedited. See [BARD-INSTRUMENTS.md](BARD-INSTRUMENTS.md) for scope and evidence.

**Next: continue class tool training with Monk (#117), splitting/linking a bounded
child before coding, or resume #80 if Q23 has been answered.** Read the source's
Artisan's Tools list and Monk entitlement; don't infer its catalog from Bard.
Reuse approved Training checkbox controls and existing tool grant/check services.
Capture a real 0.6.31 prior-writer fixture before production changes. Druid's fixed
Herbalism Kit remains another missing starting tool entitlement (#151).

#213 is closed: all twelve starting class skill lists and valid-selection
preservation are implemented. See [class skills](CLASS-SKILLS.md).

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

#214 verification: 41 native/tool checks and 16 Godot runtime checks (plus seven
native prerequisites) pass. Final added combat/source-forgery checks pass in the
training target. Main and demo extensions build; 788 localization messages validate.
The graphical main Training flow passes, including completed Bard sheet sources.
English/Spanish controls inspected at both sizes: `/tmp/opengold-bard-main-renders`.
Logs: `/tmp/opengold-bard-regression.log`, `/tmp/opengold-bard-main-render.log`,
`/tmp/opengold-bard-final-native.log`. Demo Training reached the translated-label
assertion because demo has no locale catalog; main graphical/localized run passes.
No live builds/tests remain. Historical verification stays in feature documents.
