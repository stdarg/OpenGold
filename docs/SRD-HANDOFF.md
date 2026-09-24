# SRD handoff

Updated 2026-09-24. SRD goal active; full scope remains all twelve classes
through level 4, then level 20 and multiclassing.

## Completed batch A — #29/#189 Review Training

Baseline `6b2e194`, branch `main`; delivered by the commit containing this update.
Q28 approved and implemented, superseding Q11. One original requirement (#29)
and its final child (#189) completed; no new issue or prerequisite added.
Canonical completion: [coverage](SRD-COVERAGE.md); reproducible checks and behavior:
[Training](TRAINING.md#review-training-verification-29189).

| Acceptance | Evidence |
| --- | --- |
| Complete missing training; retain original choices | Native isolated editor + campaign preview/complete; indirect-pruning rejection |
| Button beside Grip; centered shared controls | Game/demo runtime checks; Q28 |
| Fixed grants, counts, Expertise, Fighting Style, keyboard | Shared controls; all-class creator and review checks |
| Apply valid choices, Cancel/Escape discard; combat blocks | Real UI; existing native rejected-operation tests |
| Preserve wounds/resources/gear/advancement; save/reload | Native comparison of actual UI save against expected campaign |
| Original #29 scope | All twelve class skills, supported background/tools/languages, duplicate sources and Expertise tested |

Verification: 44 native/tool tests; 20 registered Godot tests (31 CTest entries
including native fixtures); game/demo Review Training; all-class creation UI;
English/Spanish 1120×800 and 1920×1080 rendered inspection; localization/diff checks.
Tested tree is baseline plus this batch's changes. Logs in `/tmp/review-training-*`.
No live processes remain at delivery. No schema, rules identity or compatibility change.

Effort: goal turn began 21:59:25 UTC; first investigation timestamp 22:00:56.
Implementation/build/focused checks overlapped; exact phase durations were not
separately captured. First full game review passed by 22:14:33; registered
regressions passed by 22:16:29; demo review and all-class creator confirmed complete at 22:19:46 UTC.
Native regression runtime 12.88s; registered Godot/fixture runtime 22.69s.
The first build regenerated Godot bindings after the necessary CMake source edit.
Token delta unavailable. Trial met its two-hour checkpoint without new scope.
Use final delivery timestamp for total batch elapsed time; do not infer token savings.

## Next batch — not started

Recommended B: #30/#192/#193 rest workflow. Refresh those issue requirements,
resolve missing controls/policies together, and create a fixed batch card before
coding. Do not inherit approval from Q28. Read only [workflow](SRD-WORKFLOW.md),
[map](SRD-REPO-MAP.md), [decisions](SRD-DECISIONS.md) and the relevant section of
[batch grouping](SRD-BATCHING-REVIEW.md). Keep one batch plus at most one necessary
prerequisite. Other pending approvals: Q19–21, Q23, Q25.

Rules 0.6.40 / PC28; campaign 11; combat 13–15 / FX1–3. Later spell/class/feat
coverage remains tracked separately; completing training does not complete a class.
