# SRD handoff

Updated 2026-09-25. Goal resumed by the active goal continuation; branch `main`.
Full objective remains all SRD_improvements issues, all twelve classes through
level 4, then level 20 and multiclassing.

## Active batch — #193 / parent #30

- Authorization: resumed goal; existing #193 acceptance and Q29–34 approvals.
- Outcome: actual rest interruptions and safe continuation through campaign events
  and combat, including earned Hit Dice choices and persisted progress.
- Acceptance: Short/Long interruption rules, sleep/light/exertion boundaries,
  per-member eligibility, no duplicate recovery, rejection/RNG rollback,
  save/load/combat continuation, original camp/inn/payment restrictions, normal
  gameplay and existing approved controls. Sleeping Unconscious behavior remains
  part of the rest requirement; do not claim closure without verifying it.
- Exclusions: unresearched probabilistic original profiles; unrelated species
  features, new spells, new controls and unrelated architecture work. Report any
  new blocking scope before implementation; no automatic child issues.
- Reuse: static SRD rest transitions, CampaignParty transactions, original ECL
  host adapters, approved RestDialog and existing save flow.
- Routing: requested **gpt-6-astra / high**; actual model/effort confirmed from
  this task's current turn_context as **gpt-6-astra / high**. No delegation.
  Escalate/report after two unsuccessful fixes of the same failure; no scope
  expansion. Failed attempts: 0 at batch start.
- Timing: started 01:27:34 UTC; checkpoint 02:27:34 UTC, latest 02:57:34 UTC.
- Verification: focused rest/campaign/encounter tests during development; one
  native regression and relevant Godot rest/party/save checks on final code.
  Preserve existing save formats and prior-writer fixtures.
- Status: ECL damage/encounter adapters and committed recovery handoff implemented
  and verified. #193/#30 remain open for natural sleep and final rest scheduling.
  Q33–34 approved on 2026-09-25; do not ask them again.
  The goal tool still reports blocked; approval itself has not changed scheduler status.
  Current inspection found an additional prerequisite: Unconscious must leave
  Prone after waking and drop held items (SRD p.191). Combat currently derives
  unconsciousness/prone attack modifiers from HP == 0 and has no persistent
  prone state, stand-up command or dropped-item recovery. No gameplay code was
  changed after approval; scope/control proposal Q35 needs resolution before
  expanding these shared mechanics. Preserve the original batch clock.
- Verification completed 01:45:36 UTC: 44 native/tool checks, 22 Godot checks
  (33 entries with prerequisites) and demo rest check passed. Session 61079 exited
  0; no live verification remains. Full evidence is in REST-RESOURCES.md.
  Logs: `/tmp/rest193-native-build.log`, `/tmp/rest193-native.log`,
  `/tmp/rest193-godot.log`, `/tmp/rest193-demo-ui.log`.
- Observed interval start to verified completion: 18m02s. Investigation,
  implementation and build phase boundaries were not captured individually.
  Distinct fixture corrections were needed (event ordering, invalid HP assertion,
  missing map/image initialization and event settling); no unresolved test failure.
  Tokens/cost delta unavailable. No new issue or scope added; no issue closed.

Previous goal work delivered #192 and static-library extraction (`9526981`).
Model policy: [routing](SRD-MODEL-ROUTING.md); execution: [workflow](SRD-WORKFLOW.md).

## Delivered: #192 — player rest controls

Q29–32 are approved. Shared game/demo Godot controls now provide Camp Short/Long
Rest selection, per-member eligibility/resources, sequential Hit Die rolls,
Finish/Escape, retained Long Rest progress and permission-checked resumption.
Pending decisions use existing camp/inn saves; no combat saving. See
[rest evidence](REST-RESOURCES.md#player-rest-controls-192),
[coverage](SRD-COVERAGE.md), and [decisions](SRD-DECISIONS.md).

Scope/architecture review: existing C++20 Core/rules boundaries, Godot 4.x,
GDExtension and RAII node ownership retained. No new runtime or save format.
Rules 0.6.40 / PC28; campaign 12 with activity, otherwise 11; combat 13–15 / FX1–3.

Verification: all 44 native/tool checks and 21 registered Godot checks passed.
Actual game/demo camp/inn and nine-state save/restart routes passed, including
pending Hit Die spending through the existing save dialog. Rendered game
English/Spanish and demo English at 1120×800 and 1920×1080 were inspected.
Final notice-localization change passed the focused Godot rest check and Spanish
render again; localization check validates 863 complete messages.

Local evidence: `/tmp/rest-ui-regression.log`,
`/tmp/rest-ui-game-save-write.log`, `/tmp/rest-ui-game-save-read.log`,
`/tmp/rest-ui-demo-save-write.log`, `/tmp/rest-ui-demo-save-read.log`,
`/tmp/rest-ui-localized-check.log`, `/tmp/rest-ui-localized-render.log`,
`/tmp/rest-ui-final-captures/`. No live verification remains.

## Remaining batch B boundary — #30/#193 stay open

Native activity/persistence landed in `7f743ee`; paused-rest combat reward
correction in `d80a3f2`; camping profile guard in `aeeb3b7`.
ECL initiative/damage adapters and earned recovery handoff are now verified.
Sleeping Unconscious actors, remaining event routes and final rest scheduling
still require #193 work. See the current event-adapter increment in
[rest evidence](REST-RESOURCES.md#193-event-adapter-increment).
Do not broadly unlock equipment/training while rest is pending. Unsupported
original probabilistic camp profiles remain explicit. Resume now reuses the
original camp permission check without restarting recovery or charging an inn.

Batch baseline: `eff6089` (closed #29/#189), recorded 22:21:39 UTC; original
checkpoint 00:21:39 UTC. Active elapsed/token deltas across context boundaries
are unavailable; do not fabricate them. No new issues added for this delivery.

Questions must be numbered, plainly visible in conversation, and preceded by
`/usr/bin/afplay /System/Library/Sounds/Glass.aiff`. Do not re-ask approved
Q29–34. Q19–21/Q23/Q25 remain outside this batch. Follow
[the workflow](SRD-WORKFLOW.md) while continuing this active goal.


## Architecture follow-up

Rest decisions and semantic timing validation now live in the statically linked
SRD library, behind engine-independent rest transition hooks. Core applies
outcomes and retains transactional campaign state. Preset class ability priorities
also moved behind CharacterRules. See TECH's static library boundary section.
No gameplay/UI behavior or save format change is intended; #193 remains open.

Architecture verification: all 44 native/tool checks and 22 Godot checks passed
(33 entries including prerequisites). The standalone content test links only
`libopengold_rules_srd5.a` and `libopengold_rules.a`; its rest scenario passes
without Core/Godot. The alternate-rules campaign test also passes. Archive
verified as arm64/x86_64 static libraries. Logs: `/tmp/srd-library-native.log`,
`/tmp/srd-library-godot.log`, `/tmp/srd-library-refresh-results.log`.
The interface change required rebuilding one stale training test object; the
fresh build passes. No gameplay issues were opened or closed by this refactor.
