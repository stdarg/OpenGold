# Wizard spell choices — frozen delivery batch

- Authorization: active standing SRD goal. One owner, branch
  `codex/srd-wizard-spell-choices`; no agents/new tasks/issues.
- Existing issues: #97 owns implementation; this batch delivers #37's ordinary
  learning/book/preparation workflow. #97 retains physical books, copying,
  replacement books and its remaining lifecycle acceptance; do not close it for
  this batch. #165 retains the full missing spell catalog.
- Player outcome: separate Wizard knowledge from preparation in real creation,
  advancement, old-save completion and completed-Long-Rest controls through
  level four. Known unprepared spells stay in the book. Replacing preparation
  never learns spells. Level-up keeps existing preparation and can fill the attained capacity; it cannot
  freely replace the existing list. One Wizard cantrip may be replaced per
  completed Long Rest. Level four adds one known cantrip.
- Acceptance: SRD counts 3/3/3/4 cantrips, 6 initial +2 book spells per gained
  Wizard level, 4/5/6/7 prepared spells; acquisition-level spell eligibility,
  independent choices, no duplicates/forged grants or repeated rest entitlements,
  atomic preview/commit, cancel/back, preset choices, old knowledge retained and
  missing knowledge pending. Preserve every supported prior save, spent slots,
  Arcane Recovery, wounds, equipment, RNG and unrelated characters.
- Scope/catalog: all currently implemented Wizard choices (five cantrips;
  Magic Missile, Scorching Ray, Blindness). Preserve full SRD capacities and show
  unfillable selections pending; do not implement new spell effects or claim
  the full Wizard list complete. Existing level/class/source restrictions remain.
- Exclusions: physical book objects, copying costs/time, book replacement/loss,
  Evoker, Ritual Adept, other classes' preparation policies, new spell effects,
  level five+, multiclassing. These remain original requirements; no acceptance
  is removed from #97, #101, #98, #165 or the overall goal.
- Reuse: static SRD spell access/grant validation, generic character history,
  transactional campaign choices/rest eligibility, existing scrollable choice
  controls and shared main/demo dialogs. Core owns transactions, SRD owns counts,
  eligibility, replacement rules and outcomes. No new UI stack or framework.
- Model: requested gpt-6-astra/high retained for independent learning/preparation
  history and completed-rest migration. Actual settings unverified; no switch.
  Escalate out-of-tier decisions or two unsuccessful fixes of the same failure.
- Timing: preflight began at observed 2026-09-25 20:33:54 UTC; frozen by 20:36:35.
  Checkpoint 21:33:54, no later than 22:03:54. Do not reset for approval waits.
- Verification: capture actual 0.6.50 writer fixtures before changing persistence;
  focused access/choice/rest/legacy continuation tests, final affected native and
  Godot checks, main EN/ES and demo EN renders/input at both supported sizes.
- Primary source: SRD 5.2.1 pp.77–78, lines 6497–6500, 6553–6595 in the official
  PDF, verified 2026-09-25. Copying/physical book rules are recorded separately
  in #97 and are explicitly not being claimed as delivered here.

## Approved layout/control behavior

WIZCHOICE-1/2/3 approved by the user on 2026-09-25.

WIZCHOICE-1: Reuse the creator's Spell Choices page with separate scrollable,
counted groups for cantrips, new spellbook entries and preparation. At Wizard
level-up use the existing level/feat/Scholar page followed by a Spell Choices
page in the same window; Back retains edits and final Confirm commits the
whole advancement. Earlier knowledge stays locked; preparation at level-up keeps existing selections and adds spells up to the
attained preparation count. Presets receive pre-generated supported choices;
unsupported catalog choices stay pending instead of shrinking SRD counts.

WIZCHOICE-2: Put a 200-pixel Spellbook button at the right of the existing
Grip/Review Training row below inventory, shortening Grip to 180 pixels and
letting Review Training occupy the middle. At minimum width, x350 label64,
x420 Grip180, x610 Review276, x896 Spellbook200; preserve the row and inventory.
Open a centered 700x700 dialog listing known/prepared spells and scrollable
checkbox groups for pending cantrip/book choices, grouped by acquisition level.
Existing selections stay locked; Apply fills eligible missing knowledge;
Cancel/Escape discards edits. Preparation remains unchanged here. Combat blocks
editing. Existing wounds/resources/equipment and ordinary save controls remain.

WIZCHOICE-3: After a completed Long Rest, present each eligible Wizard's centered
700x700 spell dialog before exploration/encounter continuation. It offers the
prepared-spell checkbox list and optional Replace/With cantrip dropdowns.
Apply commits once; Keep current declines. Choices are limited to known book
spells/current Wizard cantrips, with counts and keyboard access. Canceled or
interrupted rests and Short Rests grant no preparation/replacement entitlement.

## Prior-writer capture — completed before implementation

`opengold_spell_access_tests --capture-wizard-choices` ran against unchanged
module 0.6.50 and refuses another writer version. It produced the four
`campaign-wizard-choices-level*.ogs` fixtures and the combat before/continued
pair. They contain actual attained levels, explicit three-cantrip selections,
level-two Scholar, retained unprepared book entries, wounds, spent spell slots
and spent Arcane Recovery. Campaign 11/15 and PC33/combat continuation are real
writer output, not hand-edited current saves. Existing spell-access tests passed
after capturing. Log: `/tmp/wizard-choices-capture-build.log`.
Implementation began after all three approvals. Real fixtures remain immutable.

## Implementation and compatibility

The static SRD library offers eligible choices and validates their sources,
acquisition levels, counts, preparation and replacement. Core records independent
starting choices, independent advancement learning, and a chronological list of
subsequent spell edits at the character's attained level. Rebuilding a character
interleaves those edits with advancement, including during Review Training.
This preserves preparation changes made before later level-ups.

Campaign format 16 is conditional on independent spell choices/history or a
pending completed-rest choice window. Formats 1–15 retain their original replay:
old advancement selections may have coupled learning and preparation. Current
player advancement cannot use that historical bypass. Conditional PC34 records
cantrip replacement provenance; PC1–33 retain their original grant restrictions.
A replacement retains its original entitlement level and records its actual
learning level. The real 0.6.50 fixture bytes are unchanged.

Only a completed, individually eligible Long Rest creates the generic Core
choice window. Each member may apply or decline once. Time advancement, combat
and party edits wait until it is resolved. Saving the campaign internally retains
the exact pending window; no player combat-save controls were added. Applying
spell choices changes neither vitals nor equipment, expenditure or RNG.

Existing supported spells are the entire catalog for this batch: five Wizard
cantrips, Magic Missile, Scorching Ray and Blindness. This is insufficient to fill
all SRD book/preparation entitlements. Counts retain those entitlements and the
controls explicitly display unsupported choices as pending. Physical books,
copying/replacement costs and new spell effects remain excluded.

## Verification

Independent native checks are in `tests/wizard_choices_checks.h`, run by
`opengold_spell_access_tests`. They include actual prior-writer campaign/combat
continuation, independent creation/learning/preparation, illegal-source/level
rejection, repeated-rest rejection, preview atomicity, casting after cantrip
replacement, rest edits before advancement, and Review Training replay.

`tests/wizard_choices_view_tests.gd` exercises the real game/demo controls, Back,
Cancel/Escape, keyboard selection, locked preparations, missing old-save choices,
save/reload and combat restrictions. Its saved output has a separate native
whole-campaign comparison via `--verify-wizard-ui`. The existing creator and
shared rest checks also cover the new groups and two consecutive Wizard dialogs.
Final verification results and tested revision are recorded at delivery.


Final native coverage includes all 51 native/tool checks. The eight initially
failing test targets had their old expectations updated to reflect independent preparation,
level-four cantrip learning, campaign16 and completion of the new rest choice
window; actual prior-writer fixtures were not changed. The 26 Godot runtime
checks pass with their native fixture prerequisites. English/Spanish main and
English demo interactive/render checks cover 1120×800 and 1920×1080. The final
catalog contains 922 validated translated messages. Exact game and demo UI-save
comparisons pass.

Reproduce the focused native checks with:

```bash
cmake --build build/mac-check --target opengold_spell_access_tests -j6
OPENGOLD_GAME_DIR=/Users/edmond/POOLRAD build/mac-check/opengold_spell_access_tests
```

The optional asset environment writes `wizard-choices-ui.ogs` in the configured
build directory. Run `tests/wizard_choices_view_tests.gd` with the established
Godot executable/project and `--wizard-fixture=<absolute fixture path>`, optional
`--wizard-captures=<absolute directory>` and `--wizard-output=<absolute save>`.
For the demo add `--wizard-demo`. Compare saved output using
`opengold_spell_access_tests --verify-wizard-ui <save>` with the same asset
environment. Creator checks use `tests/cantrip_view_tests.gd`; the demo accepts
`--cantrip-demo`. Shared rest checks use `tests/rest_view_tests.gd -- --rest-check`.
Run Godot instances serially because their user-data directory is shared.

Build/verification records: `/tmp/wizard-choices-release-build.log`,
`/tmp/wizard-choices-demo-release-build.log`, `/tmp/wizard-choices-release-tests.log`,
`/tmp/wizard-final-*.log` and `/tmp/wizard-rest-*.log`.

Measured phases (UTC, 2026-09-25; overlapping phases are not summed):

| Phase | Observed interval | Evidence |
| --- | --- | --- |
| Preflight / approval | 20:33:54–20:46:57 | Frozen packet, actual old-writer capture, approvals recorded before UI changes. |
| Initial implementation / focused build | 20:46:57–20:58:49 | First native build 21s; first game build 21s; focused checks passed. |
| Integrated build | 21:02:59–21:07:39 | 4m41s; changed shared headers required native target recompilation. |
| Integrated verification / corrections | From 21:07:52 | Initial 34s pass found eight stale assertions; corrected native and Godot checks passed. |
| Final integrated build | 21:11:23–21:13:45 | 2m22s, including regenerated Godot bindings following the test output-directory definition. |
| Final UI/persistence verification | 21:13:45 onward | All new main/demo routes, rest sequences, renders and whole-save comparisons. |

Two demo build invocations exposed missing shared-header include paths in
separate translation units; corrected paths then compiled successfully. No
runtime failed-fix cycle, model switch, delegation, new issue or expanded scope
was introduced. Token/cost measurement is unavailable. The original 21:33:54 checkpoint was not reset.


## Verified delivery — runtime db864c8

Tested runtime `db864c8e1c4ecb47411765088bfdc90d73eeb7cb`, committed at
21:24:10 UTC. Final affected targets rebuilt at 21:21:28–21:22:00; the combined
77-check run passed at 21:23:54 (38s). Whole-save UI comparisons passed against
this implementation. This is 50m16s from the original 20:33:54 start, including
approval wait and builds, before the unchanged 21:33:54 checkpoint. One of one
frozen original control-workflow requirements delivered; #97's excluded book
lifecycle work is not claimed complete. Coverage is recorded once in
[the ledger](SRD-COVERAGE.md#wizard-spell-learning-and-preparation-controls).
