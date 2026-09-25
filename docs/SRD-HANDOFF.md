# SRD handoff

Updated 2026-09-25 04:38 UTC. Branch `main`; goal ACTIVE. The full objective is
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

## Next work

Consult [batch candidates](SRD-BATCHING-REVIEW.md#concrete-batches-instead-of-another-vague-grouping)
and the existing issue index #186. Package C (styles/Light attacks) and bounded
Fighter progression remain candidates; preflight affected issues and missing
routes before selecting a closure batch. Do not silently implement the entire
family or create dependencies. Other-class pools/species rest exceptions and
unverified probabilistic encounter profiles retain their existing boundaries.
Static monster profiles still lack exchange metadata; older unlocated camp
records are preserved rather than teleported to a new site.

Use [workflow](SRD-WORKFLOW.md) and [model routing](SRD-MODEL-ROUTING.md): classify,
record and visibly announce the next assignment, verify execution settings,
freeze acceptance and record a new batch clock only when starting a new batch.
No agents or new tasks without explicit authorization. Commit/push owned changes.

Questions must be numbered, visible in the conversation, and preceded by
`/usr/bin/afplay /System/Library/Sounds/Glass.aiff`. Use both the async widget and
plain visible final question. Never re-ask approvals in
[the decision register](SRD-DECISIONS.md). Q19–21/Q23/Q25 remain outside the
completed rest batch. Player saving remains camping/inn only, never combat.
