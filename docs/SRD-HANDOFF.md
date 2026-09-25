# SRD handoff

Updated 2026-09-25. Goal resumed by the active goal continuation; branch `main`.
Full objective remains all SRD_improvements issues, all twelve classes through
level 4, then level 20 and multiclassing.

## Active batch — #193 / parent #30

- Goal ACTIVE; both issues OPEN. Full acceptance remains all twelve classes,
  level 4 then 20 and multiclassing. No agents, new tasks or added issues.
- Authorization: resumed goal and Q29–35. Q35 is the one approved prerequisite:
  persistent Prone, dropping held equipment and combat recovery controls.
  Q36 automatic collection after victory/safe rest is PENDING; do not implement
  it without an answer. Audio/widget/visible question already delivered.
- Acceptance: interrupted Short/Long Rest, fresh qualifying segments (Q32),
  sleep/light/exertion boundaries, per-member eligibility, transactional rejection,
  saved progress and combat continuation, original camp/inn/payment restrictions.
  Reuse approved RestDialog, Wake ally and Ground item/Pick up controls.
- Architecture: SRD decisions stay in `opengold_rules_srd5` STATIC; Core owns
  generic inventory transactions and persistence; Godot consumes legal commands.
- Routing: gpt-6-astra/high was verified and announced for this complex rules,
  inventory and save integration. No model change or delegation. Two failed fixes
  of the same failure require reporting/escalation under the routing policy.
- Timing: original batch began 01:27:34 UTC; checkpoint 02:27:34, maximum 02:57:34.
  Overruns reported at 02:49 and 02:58; do not reset this clock. Held-item increment
  began 03:08. Previous goal turns made source/test progress, not a blocked streak.
- Delivered baseline: `1f5b590` on main implements natural sleep, waking,
  persistent Prone/crawling/standing and the game/demo controls. ECL damage and
  encounter interruptions landed earlier. #193/#30 are not complete.
- Current increment: module 0.6.42, conditional combat 16, campaign 13 only when
  detached equipment exists. Sleep/zero HP drops character-profile weapons and
  shields. Combat pickup validates reach, equipment capacity and interaction/
  Action cost. Core transfers real inventory/provenance without duplication;
  detached items survive camp saves. Main/demo controls and equipment art update.
  Older saves retain recorded gear on initial load; activating the new ledger
  reconciles all already-unconscious holders before writing a new checkpoint.
- Verified before the final legacy edge correction: all 45 native/tool tests,
  all 23 Godot checks (35 with prerequisites), demo shared control check and
  English/Spanish game rendering at 1120×800 and 1920×1080. Final legacy regression
  passes. Logs `/tmp/held-integrated-{native,godot,demo-ui,render}.log`; captures
  `/tmp/held-integrated-captures`; legacy `/tmp/held-legacy-tests.log`.
- Final verification complete 03:50 UTC: eight affected native checks and
  Godot sleep passed after the legacy fix; rebuilt demo runtime/visual checks
  passed after a label-spacing correction. All 871 English/Spanish messages
  validate. No live builds/tests remain. Logs `/tmp/held-final-tests.log`,
  `/tmp/held-demo-spacing-ui.log`; final demo captures
  `/tmp/held-final-demo-captures`. Localization regeneration changed source
  references only. Architecture/scope and diff checks passed.
- Verified rest-import increment (committed with this handoff): generic module release query,
  rest-session ground records (conditional campaign 14), and explicit initial
  ground ordinals reuse combat 16. Waking before initiative no longer re-equips
  camp items. Abandoning a rest retains its items without importing them into
  unrelated encounters. Actual prior campaign 13/combat 16 fixtures captured
  from `381b2f8`; source bytes are frozen. Initial sleep/rest tests passed.
- Final rest-import checks complete 04:13 UTC; no live builds/tests. Native
  regression passed 44/45; the old Warlock oracle assumed a held Quarterstaff
  stayed equipped through sleep. Its corrected physical-item conservation check
  passed, along with affected save/rest/spell and Godot checks (7/7). All other
  registered Godot checks passed; the prior failed prerequisite was rerun.
  An initiative concern was disproved by its test: shields do not get the
  untrained-armor penalty. That unnecessary change/test was fully removed;
  final native sleep and game sleep checks passed (2/2). This was not a repeated
  unsuccessful fix of the same failure. Logs `/tmp/rest-ground-native.log`,
  `/tmp/rest-ground-godot.log`, `/tmp/rest-ground-verified-tests.log`,
  `/tmp/rest-ground-final-restore-tests.log`.
- Rebuilt demo passes shared sleep and rest controls. Logs
  `/tmp/rest-ground-demo-sleep.log`, `/tmp/rest-ground-demo-rest.log`.
  All 871 localized messages validate. This increment changes no control layout.
- Remaining: Q36 safe automatic collection, camp location/recovery after a rest
  ends or is abandoned, and final rest scheduling acceptance. Script-induced
  zero HP during an awake Short Rest still drops gear on combat entry; this
  increment addresses natural sleep, not a new world-wide ground inventory.
  Detached gear remains saved, but outside-combat pickup is not playable yet.
  Next scheduler audit: `phlan_session.cpp` still advances five minutes directly
  for the guaranteed city-watch route, without beginning a rest activity. Read
  original event semantics before adapting it; do not invent probabilistic rules.
  Static monster profiles still lack exchange metadata. #193/#30 remain OPEN.
- Delivery: rest-import source, tests and prior-writer fixtures committed/pushed
  after `381b2f8` on main. Continue within Q35; no new issues,
  delegation, model changes, automatic cleanup or silent scope expansion.

Evidence belongs in [rest resources](REST-RESOURCES.md#combat-held-equipment-increment-193-q35).
Execution: [workflow](SRD-WORKFLOW.md); [routing](SRD-MODEL-ROUTING.md).

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
