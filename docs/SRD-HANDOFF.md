# SRD handoff

Updated 2026-09-24. Goal active: close all SRD_improvements issues, preserving
all twelve classes through level 4, then level 20 and multiclassing.
Previous turn made verified progress: native rest activity pushed in `7f743ee`.
Batch A closed #29/#189 in `eff6089`; latest GitHub snapshot: 166 open. See [coverage](SRD-COVERAGE.md).

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

Delivered native increments: `aeeb3b7` corrected unsupported camping profiles;
`7f743ee` added resumable activity and format-12 persistence with real prior-writer
fixtures. Existing atomic camp/inn routes consume the engine. See
[rest evidence](REST-RESOURCES.md#resumable-activity-persistence-and-evidence) and
[coverage](SRD-COVERAGE.md) rather than repeating delivery history here.
Rules 0.6.40 / PC28; campaign 12 with activity, otherwise 11; combat 13–15 / FX1–3.
No new issues or gameplay controls. Q19–21/Q23/Q25 remain outside this batch.

Current native handoff correction: a complete combat victory during paused rest
previously threw while awarding XP. Rewards now commit with their existing
idempotency/rollback guarantees, preserving rest and invalidating stale tickets.
XP/loot are narrowly permitted; other party edits remain locked. Complete-fight,
loot, duplicate/overflow and save/reload/resumption regression passes.
Implementation/reproduction 22:52–22:58 UTC; build/verification overlapped
22:55–23:01 UTC. Final tested tree: all 44 native/tool and 20 registered Godot
checks passed (31 entries with fixtures), plus actual game/demo party/recovery
routes. Logs: `/tmp/rest-victory-final-regression.log`,
`/tmp/rest-victory-game-route.log`, `/tmp/rest-victory-demo-final-route.log`.
Verified 23:01:27 UTC; no live verification remains. Scope review: existing C++20
Core/rules boundaries and RAII retained; no UI, schema or module identity change.
No issue closes until the full batch acceptance is met. Token delta unavailable.
The prior native implementation/verification ran 22:31–22:51 UTC. Batch checkpoint
remains 00:21:39 UTC; do not reset it for this correction.

Next remains batch B: connect initiative/non-cantrip/damage events and sleeping
actors; integrate approved controls only after Q29–31 answers. The existing
city-watch five-minute path is unchanged. Reuse `phlan_session.cpp:finish_event`
and `rolf_tour.cpp:camp`; resume needs the original permission check without
starting another rest or charging inn payment twice. `award_experience`/`award_loot` now allow paused-rest rewards. `read_character` permits identical post-combat HP/wealth readback while retaining
the mutation lock. Real damage/spell event adapters still need scoped treatment. Do not broadly unlock equipment/training.
