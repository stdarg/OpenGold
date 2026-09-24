# SRD handoff

Updated 2026-09-24. Goal active: close all SRD_improvements issues, preserving
all twelve classes through level 4, then level 20 and multiclassing.
Previous turn made verified progress: batch A delivered in `eff6089`; #29/#189
closed. GitHub snapshot after delivery: 166 open. See [coverage](SRD-COVERAGE.md).

## Active batch B — #30/#192/#193 rest workflow

Baseline `eff6089`, branch `main`. First recorded 22:21:39 UTC; checkpoint 00:21:39 UTC.
No extra issue or separate prerequisite. UI approvals Q29–31 are pending; Q32 is approved (fresh qualifying segments);
continue independent native work, not dependent controls. [Decision register](SRD-DECISIONS.md).

| Fixed acceptance | Implementation owner / verification |
| --- | --- |
| Per-member eligibility, Hit Dice, partial/full recharge | Existing native recovery and campaign rest; all twelve starting classes and supported higher levels |
| Camp Short/Long choice, sequential die/results, finish, camp/inn saving | Shared game/demo rest dialog after Q29/Q30; real controls, localization and layouts |
| Track sleep/light activity and exact elapsed/rest progress | Campaign rest activity transaction; rules-owned timing/recovery; boundary tests |
| Initiative/non-cantrip/damage and exertion interruption | Campaign activity + combat/event adapters; rejected/duplicate/stale requests preserve state/RNG |
| Interrupted Long Rest benefits and immediate resumption | Once-per-earned spending entitlement, retained progress, extra hour; Q31 UI |
| Persist pending activity, individual eligibility and expenditure | Campaign save/load + combat handoff; retain all existing formats, freeze prior writer before changes |
| Original camp/inn boundaries and event rollback | ECL entry 2/3 and PROGRAM 9 adapters; verified profiles only; failed payment/event rollback |
| Ordinary game/demo acceptance | Native suite, rest UI runtime, camp/inn routes, English/Spanish minimum/normal rendering |

Authority inspected: SRD 5.2.1 pp.185/187; [rest support](REST-RESOURCES.md),
[original campaign mappings](RECOVERY.md). Native resources/spending already exist;
Native resumable activity is implemented; UI and automatic event integration remain. Original probabilistic profiles stay explicit
until researched; do not silently treat them as safe or as city-watch events.

Completed correction: `phlan_session.cpp` now rejects chance values above 101
instead of treating them as verified city-watch profiles. A regression covering 102/200/254
failed before the fix; campaign-rest, rest-resource and save tests pass afterward.
Both 100 and 101 remain supported. Log `/tmp/rest-profile-check.log`.
General rest activity must account for interruption timing and must not reuse
previously earned Short Rest benefits on resume/load. Q32 approves a fresh uninterrupted segment of at least one hour per benefit;
resuming does not credit an earlier segment again.

Entry points: `campaign_rest.cpp`, `campaign_party.h/.cpp`, `campaign_save.cpp`,
`phlan_session.cpp`, `rolf_tour.cpp`, main/demo `rolf_tour_view.cpp`.
Baseline: rebuilt and passed campaign_rest/rest_resource tests (0.92s); log
`/tmp/rest-batch-baseline.log`. No gameplay UI change yet. Record later phase
transitions explicitly; token delta unavailable.

Rules 0.6.40 / PC28; campaign 12 with activity, otherwise 11; combat 13–15 / FX1–3. Read
[workflow](SRD-WORKFLOW.md) and [map](SRD-REPO-MAP.md) for checks/navigation.
Other pending questions Q19–21/Q23/Q25 remain outside this batch.

Phase record: baseline/requirements inspected 22:21–22:25 UTC; independent
regression reproduced at 22:25:22; correction and three focused suites completed
before broad verification began. Broad verification logs:
`/tmp/rest-batch-regression.log`, `/tmp/rest-batch-demo-build.log`.
Next work stays within this batch; no issues closed for the guard correction.

Guard verification: all 44 native/tool and 20 registered Godot checks passed
(31 Godot/fixture entries); actual game and demo party/recovery routes passed. No UI changes.
Verification completed 22:30:45 UTC. No live processes remain.
Native activity implementation began 22:31:07 UTC. Preserved actual `aeeb3b7`
writer fixtures before save edits. Focused activity/save checks passed. Native implementation/review ran
22:31–22:49 UTC; build/verification overlapped 22:46–22:51 UTC. Final verification
completed 22:51:40 UTC: all 44 native/tool checks and 20 Godot checks passed
(31 entries with fixtures), plus actual game/demo party and recovery routes.
Logs `/tmp/rest-activity-final-regression.log`, `/tmp/rest-activity-game-route.log`,
`/tmp/rest-activity-demo-final-route.log`. No live verification remains.
Tested the native-activity commit tree; docs only updated afterward. Token delta
unavailable. No issues closed: this increment proves native requirements while
player controls and event integration remain pending.

Current native delivery: rules-owned timing, revisioned rest requests,
sleep/light/exertion clocks, Q32 fresh segments, abandonment, format-12 activity
persistence, prior format-11 exact next-die continuation, interrupted combat
handoff, and blocking unrelated town events while an activity is retained.
Existing atomic safe-camp/inn routes consume the engine. No new UI or issues.

Next remains batch B: connect initiative/non-cantrip/damage events and sleeping
actors; integrate approved controls only after Q29–31 answers. The existing
city-watch five-minute path is unchanged. Reuse `phlan_session.cpp:finish_event`
and `rolf_tour.cpp:camp`; resume needs the original permission check without
starting another rest or charging inn payment twice. Revisit `editable()` for
scoped interruption event writes; do not broadly unlock equipment/training.
