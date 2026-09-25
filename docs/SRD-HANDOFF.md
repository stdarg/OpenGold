# SRD handoff

Updated 2026-09-25 during the Chill Touch batch. Branch `codex/srd-chill-touch`; goal ACTIVE. The full objective is
all SRD_improvements issues, all twelve classes through level 4, then level 20
and multiclassing. Do not treat this completed batch as the full goal.

## Delivered batch B — #30/#192/#193

- #192 was already CLOSED. This delivery completes #193 and parent #30;
  their GitHub closure records link the final commit and verification evidence.
- Q33–35 deliver natural sleep/waking, persistent Prone, held-equipment dropping,
  combat pickup and inventory/save handoff. Q37 APPROVED safe standing/collection
  after victory and safe camp completion/cancellation/obeying city watch;
  it supersedes Q36. No question remains pending for this batch.
- Safe recovery stows reachable gear in the living original owner's inventory,
  or an able survivor's when the owner died. Combat waking/standing/pickup costs
  stay unchanged. No healing, recharge or RNG draw accompanies safe cleanup.
- The verified original ECL3.DAX/11 city-watch route now begins/resumes actual
  rest, spends five minutes, explicitly wakes members, then ends camping on GO.
  Failed continuation rolls back. Unknown probabilistic profiles/FIGHT remain
  explicit unsupported boundaries; no new encounter scheduler was invented.
- Architecture: SRD eligibility, standing and terrain reach stay in the STATIC
  rules library. Core stages inventory and vitality transactions; no SRD link
  from Core, new runtime, UI controls or schema changes. Module 0.6.42, PC28,
  conditional combat16 and campaign14 remain compatible with prior writers.
- Final checks: 45/45 native/tool tests; 22/22 Godot runtime checks (34 including
  native prerequisites); rebuilt demo sleep/rest checks; nine original-data
  save/write/restart states in BOTH game and demo; 871 localized messages valid.
  Tests cover living/dead owners, blocked paths, repeated collection, save/load,
  watch rollback, fresh segments, spent resources, mixed eligibility and time.
- Evidence: [rest resources](REST-RESOURCES.md#safe-recovery-and-city-watch-completion-193-q37),
  [coverage](SRD-COVERAGE.md). Logs `/tmp/safe-recovery-native.log`,
  `/tmp/safe-recovery-godot.log`, `/tmp/safe-recovery-game-save-{write,read}.log`,
  `/tmp/safe-recovery-demo-{sleep,rest,save-write,save-read}.log`.
- No live builds/tests remain. Full verification finished 04:38 UTC. Original
  batch start remains 01:27:34 UTC, checkpoint 02:27:34, maximum 02:57:34;
  previously reported overruns were not reset. Last observed phase checkpoints:
  focused fixes 04:28; integrated rebuild 04:33; verification done 04:38.
- Routing: gpt-6-astra/high retained for complex rules/inventory/save integration.
  One incorrect compact-resource test expectation was corrected from the actual
  serializer; all checks now pass. No repeated two-fix failure or model switch.
  No agents, new tasks, added issues or silent scope expansion.

## Current independent batch — Chill Touch under #165

- WIP on `codex/srd-chill-touch`, based on main `0feefe4`; do not merge or close
  an issue as complete yet. Main retains delivered safe recovery (#30/#193).
- This turn made PROGRESS: implemented and tested ordinary Wizard/Sorcerer/
  Warlock access, casting, damage, sourced healing prevention, expiry, saves,
  healing actions and presentation. Details/evidence: [packet](CHILL-TOUCH.md).
- Q40 is PENDING: interpretation of a Stable natural recovery deadline reached
  while healing is blocked. Audio and widget delivered; visible final question
  must accompany this handoff. Do not infer approval or implement either policy.
  `advance_recovery_clock` still ignores prevention: this is known incomplete
  gameplay on the WIP branch, not delivered support. Keep #165/#35 open.
- Q38/Q39 remain pending; Medicine/Tactical Mind stays deferred. No other batch
  may silently replace this WIP. No new issues, agents, tasks or model switch.
- Routing remains gpt-6-astra/high (complex healing/mortality/timeline/save work),
  inherited verified execution setting. Start 04:56:05, checkpoint 05:56:05,
  maximum 06:26:05 UTC; do not reset. Implementation/focused integration by
  05:19, then final independent verification. No two-fix failure escalation.
- Existing Q18 is MAIN-GAME Spell/Cast approval. Demo shares character creation
  but explicitly retains its older combat controls (CANTRIP-CONTROLS.md).
  Do not add demo combat controls under an incorrect shared-layout assumption.
- Persistence: module 0.6.43; PC29 only when Chill Touch is selected, PC28 otherwise;
  conditional FX5 for active block plus posture. Actual 0.6.42/PC28 and held-item
  fixtures remain unchanged and their compatibility tests pass. No Core SRD logic.
- Independent verification finished 05:23:42 UTC (27m37s elapsed): 46 native/tool
  checks have passing results; 23/23 Godot runtime checks; rendered combat and
  main creator EN/ES at both sizes; rebuilt demo creator passes in its existing
  English-only presentation. No live process remains. Details/logs in packet.
- Preserve this WIP branch until complete; return verified work to main under
  the existing delivery workflow. No finished coverage row or issue closure yet.
- Next after Q40: integrate Stable recovery across chronological combat/campaign
  boundaries; preserve due recovery across save/load if the proposed policy is
  approved; test exact shared deadlines and partition-invariant RNG. Then verify
  final integration, update coverage/inventory, and deliver the bounded increment.
  Species/feat/object/higher-level routes stay explicitly incomplete in #165.

## Deferred batch — #32 / #87

- Previous goal turn: PROGRESS, pushed preflight/approval packet `f7b069f`;
  the preceding `36e574c` delivery closed #193/#30.
  Worktree was clean at this turn's entry; no live process remains.
- Current preflight selected Medicine stabilization and Tactical Mind together;
  [bounded packet](MEDICINE-TACTICAL-MIND.md) records fixed acceptance, sources,
  routes, code map, verification and exclusions. No gameplay changes yet.
- Q38/Q39 are PENDING, delivered with successful Glass audio, async widget and
  visible final questions. Do not implement the proposed controls until answered.
- Routing: retained gpt-6-astra/high for shared pending-check, provenance and save
  integration. No switch or delegation. Original new-batch start 04:41:34 UTC;
  checkpoint 05:41:34, maximum 06:11:34. Do not reset on the approval wait.
- Package C preflight found Light attacks require additional two-held-weapon and
  draw/stow support; style feat closure also needs Paladin/Ranger source routes.
  Deferred that larger package instead of silently expanding into it. Q23 and
  other pre-existing questions remain pending; they are not this batch's blockers.
- Check-path audit and actual prior-writer capture are now complete; see the
  packet's verified audit. There are modifier queries but no rolled SRD skill
  actions yet; initiative has no failure DC. Reuse existing d20 and stabilize
  helpers, keeping original ECL probability branches distinct.
- `opengold_action_surge_tests` now verifies three frozen 0.6.42/PC28 fixtures:
  level-two Fighter with spent Second Wind, campaign time/RNG, and exact
  Action Surge/Dash continuation. Focused build/test passes; no gameplay change.
  Log `/tmp/mind-baseline-tests.log`. No live processes remain.
- Next: after Q38/Q39 approval implement the two issues' player flow, with grants,
  pending decisions and old/new continuations. Full goal remains ACTIVE. No
  issue closure is claimed for preparation; no repeated-blocker streak yet.

Use [workflow](SRD-WORKFLOW.md) and [model routing](SRD-MODEL-ROUTING.md): classify,
record and visibly announce the next assignment, verify execution settings,
freeze acceptance and record a new batch clock only when starting a new batch.
No agents or new tasks without explicit authorization. Commit/push owned changes.

Questions must be numbered, visible in the conversation, and preceded by
`/usr/bin/afplay /System/Library/Sounds/Glass.aiff`. Use both the async widget and
plain visible final question. Never re-ask approvals in
[the decision register](SRD-DECISIONS.md). Q19–21/Q23/Q25 remain outside the
current Medicine/Tactical Mind batch; Q38/Q39/Q40 await answers. Player saving remains camping/inn only, never combat.
