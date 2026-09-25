# SRD handoff

Updated 2026-09-24. **Pause after delivery of #192, at the user's explicit request.**
The user subsequently authorized a bounded static-library architecture refactor.
That refactor does not resume the SRD backlog; do not start #193 or another issue. The broader goal
remains all SRD_improvements issues, all twelve classes through level 4, then
level 20 and multiclassing. Branch: `main`.

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
Automatic initiative/non-cantrip/damage/exertion event connections, sleeping
Unconscious actors, and rest scheduling still require #193 work. The controls
expose retained activity; they do not complete that automatic integration.
Do not broadly unlock equipment/training while rest is pending. Unsupported
original probabilistic camp profiles remain explicit. Resume now reuses the
original camp permission check without restarting recovery or charging an inn.

Batch baseline: `eff6089` (closed #29/#189), recorded 22:21:39 UTC; original
checkpoint 00:21:39 UTC. Active elapsed/token deltas across context boundaries
are unavailable; do not fabricate them. No new issues added for this delivery.

Questions must be numbered, plainly visible in conversation, and preceded by
`/usr/bin/afplay /System/Library/Sounds/Glass.aiff`. Do not re-ask approved
Q29–32. Q19–21/Q23/Q25 remain outside this batch. Follow
[the workflow](SRD-WORKFLOW.md) after explicit resumption.


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
