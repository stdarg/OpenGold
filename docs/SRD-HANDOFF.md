# SRD handoff

Updated 2026-09-24. **SRD goal active again.** The user resumed the goal after
installing the efficiency workflow. Work continues on #205.

## Current state

- Goal: close all `SRD_improvements` issues; all twelve classes, first through
  level 4, then through 20 and multiclassing. [Index](https://github.com/stdarg/OpenGold/issues/186),
  [plan](SRD-IMPLEMENTATION.md), [coverage](SRD-COVERAGE.md).
- Branch at handoff: `main`. Latest feature: Action Surge, commits `6b798ec`
  and `ad63d56`, pushed; [#86](https://github.com/stdarg/OpenGold/issues/86) closed.
  Last observed open label count: 162; counts are snapshots, not completion proof.
- Rules 0.6.24 / PC13; combat 14 with eligible Fighters, otherwise 13; spent
  Surge uses SRD8; campaign 11. See [Action Surge](ACTION-SURGE.md).
- The approved shared cantrip dropdown/Cast control is delivered as the first
  UI increment for #205; Ray of Frost mechanics and its creation choice remain
  unimplemented. No unfinished feature edits or live processes at handoff.

## Current issue

[Ray of Frost #205](https://github.com/stdarg/OpenGold/issues/205): read its full
acceptance before coding. High effort is appropriate for sourced, nonstacking
Speed reduction, caster-turn expiry, movement already spent and persistence.
Do not treat a native-only path as final completion.

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
  Combat selector delivered; Ray of Frost choice remains to implement. No new
  layout question is needed for that already-approved choice.
- Play Glass.aiff before numbered questions. Batch related questions and reuse
  approvals. Existing scope/architecture rules still apply.

## Verification and local environment

- Shared selector passed 9 affected Godot checks, 6 native fixture prerequisites,
  English/Spanish rendering at both sizes and 739-message localization validation.
- Action Surge native implementation passed 38 native/tool and 15 Godot runtime checks.
  Button follow-up passed 8 affected Godot checks, 5 native fixture prerequisites,
  rendered English/Spanish at both supported sizes and 739-message localization.
  These results cover Action Surge, not future edits. [Evidence](https://github.com/stdarg/OpenGold/issues/86#issuecomment-5818117420).
- Bash/macOS; `build/mac-check` is the game/native build; `build/sprite-demo` is
  separate. Godot: `/Applications/Godot_mono.app/Contents/MacOS/Godot`.
- Main project: `src/OpenGoldBox/godot`; local original assets: `/Users/edmond/POOLRAD`.
  Use [the workflow](SRD-WORKFLOW.md) for focused build/test commands and boundaries.
