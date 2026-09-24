# SRD handoff

Updated 2026-09-24. **SRD goal active again.** The user resumed the goal after
installing the efficiency workflow. Work continues on #205.

## Current state

- Goal: close all `SRD_improvements` issues; all twelve classes, first through
  level 4, then through 20 and multiclassing. [Index](https://github.com/stdarg/OpenGold/issues/186),
  [plan](SRD-IMPLEMENTATION.md), [coverage](SRD-COVERAGE.md).
- Branch: `main`. Rules 0.6.25 Ray of Frost Wizard increment is delivered; shared selector was delivered in `bc2d152`. #205 remains partial.
  Last observed open label count: 162; this is a snapshot, not completion proof.
- PC14 / FX2; combat 15 for Ray access/effects, otherwise 13/14; campaign 11.
  [Ray of Frost evidence and remaining scope](RAY-OF-FROST.md).
- Wizard choice, main-game casting, Cold damage, nonstacking sourced slow,
  caster-turn expiry and campaign/camp continuation are implemented.

## Current issue

Concentration transition foundation #207 is delivered; [evidence](CONCENTRATION.md).
Next: [Silence combat/area/player integration #208](https://github.com/stdarg/OpenGold/issues/208),
then [campaign/ritual integration #209](https://github.com/stdarg/OpenGold/issues/209).
Both link back to #38/#39/#43/#174. Read acceptance before implementation.
The isolated CN1 concentration subrecord now passes canonical, malformed-input
and deterministic continuation tests; it is not yet embedded in live saves.
Questions 19–21 are pending; do not implement their dependent choices until answered:
- Q19: explicit flat-grid adaptation: choose a square center, circular 20-foot
  radius, whole occupied square for full containment, walls block spread, preview
  distinguishes partial squares/fully contained creatures; no height/airborne model.
- Q20: prepared “Silence — level 2” in existing Spell dropdown; Cast enters area
  preview, arrows move it, Enter/click commits, Escape cancels free. New row below
  Dash shows End concentration, spell and duration; release costs no action and
  is available for the selected owner outside their turn.
- Q21: Silence in current level 3–4 Cleric preparation choices, retaining current
  limits/confirmation; explicit choice only, no change to saved preparations.
These requests cite AGENTS.md for controls and explicit review for geometry.
#205 remains partial for speech blockers and the other linked granting sources.
Mundane gagging also remains under #39; do not equate it with magical silence.

Relevant entry points (read only the needed sections):

- `src/OpenGold.Rules.Srd5/src/{srd5.cpp,status_effects.h,status_effects.cpp}`:
  legal commands, attack resolution, movement, effect lifecycle and checkpoint.
- `src/OpenGold.Rules.Srd5/src/{spell_access.h,spell_access.cpp,spell_components.h}`:
  sourced access and components. [Spell inventory](SPELL-INVENTORY.md).
- `src/OpenGoldBox/{combat_view.cpp,character_creation_view.cpp}` and game scenes:
  existing controls. [Spell access](SPELL-ACCESS.md), [status effects](STATUS-EFFECTS.md).
- `tests/{poison_spray_tests.cpp,sacred_flame_tests.cpp,status_effect_tests.cpp}`
  and `tests/{poison_view_tests.gd,sacred_view_tests.gd}`: reusable test patterns.

## Decisions to preserve

- No player combat saving. Saving is at camp or an inn; internal checkpoints
  are allowed for deterministic continuation/testing.
- Missing old-save training choices stay pending for Review Training (#189).
  Preset characters get pre-generated choices. Question 11 about the specific
  Review Training layout is still unresolved; retrieve its wording if needed.
- Creation has approved Training and Spell Choices steps; Back retains valid
  selections and missing catalog choices stay explicitly pending.
- Grip dropdown, HP source tooltips/colors, Temporary HP replacement, Adrenaline
  Rush, Savage Attacker's two-stage dialog and Action Surge controls are approved.
  See their feature docs/current implementation rather than re-asking.
- **Question 18 approved 2026-09-24:** replace individual combat cantrip buttons
  with a labeled Spell dropdown and Cast button in the same row to the right of
  Adrenaline Rush. List known cantrips; Cast highlights legal targets; clicking
  casts. Support keyboard use and retain A/Space; disable unavailable Cast.
  Ray of Frost becomes a Wizard choice in the existing Spell Choices step.
  Combat selector and Ray of Frost choice are delivered. Reuse this approval.
- Play Glass.aiff before numbered questions. Batch related questions and reuse
  approvals. Existing scope/architecture rules still apply.

## Verification and local environment

- Ray increment: 39 native/tool checks pass in aggregate (only stale catalog count
  failed the broad run; corrected and rerun). 16 Godot runtime checks plus seven
  native fixture prerequisites pass. Main/demo builds and 744-message localization
  check pass. Rendered combat at both supported sizes in English/Spanish.
- Wizard and Cleric creation checks cover the shared step; see the feature doc.
- Concentration foundation, status effects and Ray of Frost focused checks pass.
  CN1 serialization follow-up passed the rebuilt concentration test.
  No runtime/save changes in this helper increment.
- No remaining live processes after final verification. No UI questions needed
  for the delivered Ray controls; Review Training question 11 remains unresolved.
- Bash/macOS; `build/mac-check` is the game/native build; `build/sprite-demo` is
  separate. Godot: `/Applications/Godot_mono.app/Contents/MacOS/Godot`.
- Main project: `src/OpenGoldBox/godot`; local original assets: `/Users/edmond/POOLRAD`.
  Use [the workflow](SRD-WORKFLOW.md) for focused build/test commands and boundaries.
