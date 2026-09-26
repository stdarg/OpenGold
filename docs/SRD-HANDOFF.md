# SRD handoff

Updated2026-09-26. Standing goal **ACTIVE and incomplete**: close all
`SRD_improvements`, preserving all twelve classes through4 and planned higher
levels/multiclassing. Do not pause at delivery boundaries or ask for a new resume.
A decision reply alone never resumes a genuinely paused goal. No agents/new tasks
or model/runtime setting changes are authorized.

## Latest delivery — Light attacks

- Branch `codex/srd-light-attacks`; runtime/test **69771eb**, rules0.6.55.
  **#59 Light, #81 Two-Weapon Fighting, #56 Loading are CLOSED** after push.
  **145 open SRD issues**, verified in `/tmp/srd-open-after-light.json`.
- [Coverage](SRD-COVERAGE.md#light-attacks-two-weapon-fighting-and-loading-598156)
  is canonical. [Packet](LIGHT-ATTACKS.md#delivery-and-verification) records frozen
  acceptance, compatibility, exact checks, limits and timings. Main remains375f58f;
  no PR/main integration is claimed.
- Actual two-hand equipment, ordinary/reaction weapon selection, different-weapon
  Light Bonus Action attacks, TWF damage/source routes, and Loading action
  boundaries work through current level1–4 paths. Nick/mastery, Extra Attack,
  other styles and remaining class/spell work remain in their original issues.
- All78 integrated checks pass after fresh50-target rebuild; mainEN/ES+demoEN
  combat and three-class advancement pass at both sizes. Native combat-state and
  six campaign-save comparisons pass;960 translations validate. Real0.6.53/0.6.54
  historical captures retained. An integration failure caught non-thrown Light
  identities initialized too late; fixed at new encounter creation and reverified.
- Preflight00:45:02, checkpoints recorded without resetting, runtime commit
  03:38:15 UTC:2h53m13s elapsed including approvals/unobserved interval. Three
  original requirements delivered, zero new issues. No token/speedup claim.
- LIGHT-1/2/3 were approved by “2. Yes. 3. Yes” and “1. Yes”. No pending
  questions, live builds/tests, or code WIP for this batch. Do not ask again.
- Requested Astra/high for this batch's interactions/history; actual configured
  model/effort unverified. No model change. Apply routing before the next batch.

## Next action

Preflight the next bounded delivery from the existing
[batching review](SRD-BATCHING-REVIEW.md), using the current145-issue snapshot.
Freeze player outcome, acceptance, included issues and exclusions before coding.
No automatic child tickets or speculative prerequisite work. Preserve full scope
of parent#85/#140/#147 and all twelve classes. Use the repo map for targeted reads.
Prior Fighting Style delivery#78/#79/#80 is recorded in coverage, not repeated here.

## Preserved decisions and exclusions

#97 remains physical spellbooks/copying; #165 retains missing spells. #98/#101
and the full Wizard tracker remain open. Two Evocation spells do not satisfy all
Savant learning. Q19–21 remain pending; Q23 is superseded by approved STYLE-1.
Unlimited ammunition is AMMO-UNLIMITED; do not reopen it. Camp/inn saves only.
Consult `SRD-DECISIONS.md` before questions; reuse approvals within recorded scope.

## Persistent execution agreements

Read `SRD-WORKFLOW.md`, `SRD-MODEL-ROUTING.md`, `SRD-REPO-MAP.md` and relevant
requirements. One owner/batch; no agents/tasks/settings changes/silent scope growth.
Rules stay in static SRD library; Core orchestrates; UI presents rule-owned choices.
No build-input edits during builds. Rebuild before testing; Godot runs serially.
Focused checks, then final integrated evidence; preserve all released-save history.
Commit/push only owned verified work. Coverage is canonical; handoff stays compact.

Questions restart at1. Play `/usr/bin/afplay /System/Library/Sounds/Glass.aiff`,
verify exit0, and write each full question visibly; widget alone is insufficient.
Do not ask repeatedly for routine choices already within accepted scope.
