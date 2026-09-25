# Arcane Recovery — #99 delivery packet

## Frozen batch

- Authorization: standing SRD goal. One owner/batch, #99 only, branch
  `codex/srd-arcane-recovery`. No new issues, agents or tasks.
- Current behavior: Wizards have no Arcane Recovery implementation. Ordinary
  Wizard creation/advancement already reaches levels 1–4; existing Short Rest
  recovery controls and camp/inn saves provide the integration path.
- Player outcome: at a completed Short Rest, an eligible Wizard may choose
  spent spell slots with combined levels at most half Wizard level rounded up.
  Spend one Arcane Recovery use only on a valid nonempty choice; restore the use
  on Long Rest. Preserve resources, wounds, training and equipment otherwise.
- Acceptance: attained Wizard levels 1–4, fixed source grant, capacities and
  missing-slot limits, Short Rest eligibility/ticket validation, once-per-Long-Rest
  use, no partial/repeated transaction exploits, rejected-command atomicity,
  current/old campaign and combat continuation, rest interruption benefits and
  save/load. Actual main/demo UI, keyboard, EN/ES at both supported sizes.
- Exclusions: Scholar, Ritual Adept, Evoker, additional spell catalogs,
  spellbook/preparation redesign, other classes' recovery features and later
  level/multiclass progression. No player combat saving.
- Reuse: Q29–31 rest dialog and transactional Short Rest spending; existing
  resource pools, SRD grant provenance and opaque vitals. Core owns transactions
  and clock; statically linked SRD library owns eligibility, selection outcomes
  and resource arithmetic; shared Godot rest dialog presents choices.
- Routing: requested `gpt-6-astra` / `high`, retained for rest-state/persistence
  interactions. Actual configuration not independently verified; no switch.
  Escalate an out-of-tier decision or two unsuccessful same-failure fixes.
- Verification: capture actual 0.6.48 writer evidence before runtime changes;
  focused feature/rest/save checks, final native regression, affected rest UI
  and rendered main/demo EN/ES. Never regenerate frozen old saves with new code.
- Timing: investigation began 2026-09-25 18:57:23 UTC; checkpoint 19:57:23,
  maximum 20:27:23. Retain these times across questions and context resets.

## Source

[SRD 5.2.1, printed page 78](https://media.dndbeyond.com/compendium-images/srd/5.2/SRD_CC_v5.2.1.pdf):
level-one Wizard feature, recovery chosen when a Short Rest finishes; combined
slot levels at most half Wizard level rounded up, no slot of level six or higher;
one use per Long Rest. Thus supported levels 1–2 recover at most one level-one
slot; levels 3–4 may recover one/two level-one slots or one level-two slot.
Using a smaller legal combination still consumes the feature's use.

## Pending control decision — AR-1

Visible question 1 proposes a labeled Arcane Recovery dropdown and Recover slots
button in the existing Rest window, above Result, shortening the scrollable Info
area. Show legal combinations for the selected eligible Wizard after Short Rest;
commit slots/use immediately on Recover slots. Finish/Escape without use preserves
the use. Keyboard controls and existing camp/inn saving. Current window is 720×640;
Info occupies y274–482, Result y490–560, action buttons y580–620, so the proposed
row fits within that existing window by shortening Info. Shared main/demo code.
This proposal is not approval. Sound played before asking. No dependent UI edits.

## Native implementation (UI pending)

The SRD library now supplies the fixed grant, legal opaque recovery choices,
slot allocation, once-per-Long-Rest resource, and validation. Core validates the
completed Short Rest ticket and member eligibility, then commits a candidate
party state atomically. It contains no Wizard or spell-slot arithmetic.

Module 0.6.49 adds PC32 grant provenance, SRD9 spent-use vitality and combat
format 20. Unspent states preserve previous compact encodings. Real 0.6.48
campaign fixtures cover attained levels 1–4; actual pre-change combat bytes prove
unchanged old continuation. Their provenance and hashes are in
[fixture documentation](../tests/fixtures/README.md). No frozen fixture was
regenerated with the new writer.

`arcane_recovery_tests.cpp` covers legal/over-budget choices, stale tickets,
nonmembers, duplicate use, full pools, sleeping eligibility, rejected atomicity,
Long/Short Rest recharge differences, advancement, save/reload, combat with and
without physical inventory, and interrupted Long Rest benefits. A specific
regression ensures lasting effects continue to expire in SRD9 vitality without
refreshing Arcane Recovery. Old identities reject new grants/resource records.

Initial feature/grant/rest checks passed (3/3); the expanded feature test passed.
Final native/tool regression passes 51/51; this is not a completed playable delivery.
AR-1 remains pending: no new UI controls have been implemented and #99 stays open.

| Phase | Observed UTC | Evidence |
| --- | --- | --- |
| Batch start | 18:57:23 | Frozen scope and pending AR-1; original checkpoint retained. |
| Native review after context restoration | 19:14:39 onward | Initial checks verified; additional effect-timer regression added and passed. |
| Native regression build | 19:16–19:21 UTC (minute precision) | All 49 native test targets built. |
| Regression corrections and verification | 19:21–19:23:42 UTC | Six previous-profile assertions updated; two expected old-save migrations include the exact fixed grant. Final 51/51 pass. |

Earlier capture/implementation phase endpoints were not recorded precisely;
no reconstructed durations or token-saving claim. The checkpoint remains
19:57:23 UTC, maximum 20:27:23 UTC.

Verified commands on native runtime/test revision `bc948e7` (module 0.6.49):

- `cmake --build build/mac-check --target <49 native test targets> -j6`
  (targets selected from CTest executable paths; no Godot packaging).
- Rebuilt each changed test target after correcting expected profile versions
  and exact historical grant migrations.
- `ctest --test-dir build/mac-check --output-on-failure -E '^opengold_godot_' -j6`
  — 51/51 pass; final log `/tmp/arcane-native-final.log`, 1.49 seconds.
- `python3 tools/localization.py --check` — 901 English/Spanish messages valid.
- `git diff --check` — clean. Fixed old-writer SHA-256 hashes rechecked unchanged.

Native verification ended 19:23:42 UTC, 26m19s after the original batch start.
No issue closed; player controls/rendering are still pending AR-1. No live build
or test handle remains. No model switch, agents, new issues or expanded scope.
